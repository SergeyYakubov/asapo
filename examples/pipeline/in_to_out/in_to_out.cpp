#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <mutex>
#include <string>
#include <sstream>


#include "asapo_worker.h"
#include "asapo_producer.h"

using std::chrono::system_clock;
using asapo::Error;

std::string group_id = "";
std::mutex lock;

struct Args {
    std::string server;
    std::string file_path;
    std::string beamtime_id;
    std::string stream_in;
    std::string stream_out;
    std::string token;
    int timeout_ms;
    int nthreads;
    bool transfer_data;
};

void ProcessAfterSend(asapo::GenericRequestHeader header, asapo::Error err) {
    if (err) {
        std::cerr << "Data was not successfully send: " << err << std::endl;
        return;
    }
}


void WaitThreads(std::vector<std::thread>* threads) {
    for (auto& thread : *threads) {
        thread.join();
    }
}

int ProcessError(const Error& err) {
    if (err == nullptr) return 0;
    std::cout << err->Explain() << std::endl;
    return err == asapo::IOErrorTemplates::kTimeout ? 0 : 1;
}

std::vector<std::thread> StartThreads(const Args& args, asapo::Producer* producer,
                                      std::vector<int>* nfiles,
                                      std::vector<int>* errors) {
    auto exec_next = [&args, nfiles, errors, producer ](int i) {
        asapo::FileInfo fi;
        Error err;
        auto broker = asapo::DataBrokerFactory::CreateServerBroker(args.server, args.file_path,
                      asapo::SourceCredentials{args.beamtime_id, args.stream_in, args.token}, &err);

        broker->SetTimeout((uint64_t) args.timeout_ms);

        lock.lock();

        if (group_id.empty()) {
            group_id = broker->GenerateNewGroupId(&err);
            if (err) {
                (*errors)[i] += ProcessError(err);
                return;
            }
        }

        lock.unlock();

        if (i == 0) {
            auto meta = broker->GetBeamtimeMeta(&err);
            if (err == nullptr) {
                std::cout << meta << std::endl;
            } else {
                std::cout << "Cannot get metadata: " << err->Explain() << std::endl;
            }
        }
        while (true) {
            asapo::FileData data;
            err = broker->GetNext(&fi, group_id, args.transfer_data ? &data : nullptr);
            if (err) {
                (*errors)[i] += ProcessError(err);
                if (err == asapo::IOErrorTemplates::kTimeout) {
                    break;
                }
            }

            asapo::EventHeader header{fi.id, fi.size, fi.name, fi.metadata};

            Error err_send;
            if (args.transfer_data) {
                header.file_name += "_" + args.stream_out;
                err_send = producer->SendData(header, std::move(data), asapo::kDefaultIngestMode, ProcessAfterSend);
            } else {
                header.file_name = args.file_path + "/" + header.file_name;
                err_send = producer->SendData(header, nullptr, asapo::IngestModeFlags::kTransferMetaDataOnly, ProcessAfterSend);
                std::cout << err_send << std::endl;

            }

            if (err_send) {
                std::cout << "Send error: " << err_send->Explain() << std::endl;
            }

            (*nfiles)[i]++;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < args.nthreads; i++) {
        threads.emplace_back(std::thread(exec_next, i));
    }
    return threads;
}

int ProcessAllData(const Args& args, asapo::Producer* producer, uint64_t* duration_ms, int* nerrors) {
    asapo::FileInfo fi;
    system_clock::time_point t1 = system_clock::now();

    std::vector<int> nfiles(args.nthreads, 0);
    std::vector<int> errors(args.nthreads, 0);

    auto threads = StartThreads(args, producer, &nfiles, &errors);
    WaitThreads(&threads);

    int n_total = std::accumulate(nfiles.begin(), nfiles.end(), 0);
    *nerrors = std::accumulate(errors.begin(), errors.end(), 0);

    system_clock::time_point t2 = system_clock::now();
    auto duration_read = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    *duration_ms = duration_read.count();
    return n_total;
}

std::unique_ptr<asapo::Producer> CreateProducer(const Args& args) {
    asapo::Error err;
    auto producer = asapo::Producer::Create(args.server, args.nthreads,
                                            asapo::RequestHandlerType::kTcp,
                                            asapo::SourceCredentials{args.beamtime_id, args.stream_out, args.token }, &err);
    if(err) {
        std::cerr << "Cannot start producer. ProducerError: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->EnableLocalLog(true);
    producer->SetLogLevel(asapo::LogLevel::Debug);
    return producer;
}


int main(int argc, char* argv[]) {
    asapo::ExitAfterPrintVersionIfNeeded("GetNext Broker Example", argc, argv);
    Args args;
    if (argc != 10) {
        std::cout << "Usage: " + std::string{argv[0]}
                  + " <server> <files_path> <beamtime_id> <stream_in> <stream_out> <nthreads> <token> <timeout ms> <transfer data>"
                  <<
                  std::endl;
        exit(EXIT_FAILURE);
    }
    args.server = std::string{argv[1]};
    args.file_path = std::string{argv[2]};
    args.beamtime_id = std::string{argv[3]};
    args.stream_in = std::string{argv[4]};
    args.stream_out = std::string{argv[5]};
    args.token = std::string{argv[6]};
    args.nthreads = atoi(argv[7]);
    args.timeout_ms = atoi(argv[8]);
    args.transfer_data = atoi(argv[9]) == 1;

    auto producer = CreateProducer(args);

    uint64_t duration_ms;
    int nerrors;
    auto nfiles = ProcessAllData(args, producer.get(), &duration_ms, &nerrors);
    std::cout << "Processed " << nfiles << " file(s)" << std::endl;
    std::cout << "Successfully: " << nfiles - nerrors << std::endl;
    std::cout << "Errors : " << nerrors << std::endl;
    std::cout << "Elapsed : " << duration_ms << "ms" << std::endl;
    std::cout << "Rate : " << 1000.0f * nfiles / (duration_ms - args.timeout_ms) << std::endl;


    std::this_thread::sleep_for(std::chrono::milliseconds(1000));


    return nerrors == 0 ? 0 : 1;
}
