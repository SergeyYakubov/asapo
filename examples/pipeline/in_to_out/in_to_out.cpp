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

#include "asapo_consumer.h"
#include "asapo_producer.h"
#include "preprocessor/definitions.h"

using std::chrono::high_resolution_clock;
using asapo::Error;
using BrokerPtr = std::unique_ptr<asapo::DataBroker>;
using ProducerPtr = std::unique_ptr<asapo::Producer>;
std::string group_id = "";
std::mutex lock_in, lock_out;

int files_sent;
bool streamout_timer_started;
high_resolution_clock::time_point streamout_start;
high_resolution_clock::time_point streamout_finish;

struct Args {
    std::string server;
    std::string file_path;
    std::string beamtime_id;
    std::string stream_in;
    std::string stream_out;
    std::string token;
    int timeout_ms;
    int timeout_ms_producer;
    int nthreads;
    bool transfer_data;
};

void ProcessAfterSend(asapo::RequestCallbackPayload payload, asapo::Error err) {
    if (err && err != asapo::ProducerErrorTemplates::kServerWarning) {
        std::cerr << "Data was not successfully send: " << err << std::endl;
        return;
    }
    lock_out.lock();
    files_sent++;
    streamout_finish = max(streamout_finish, high_resolution_clock::now());
    lock_out.unlock();

}


void WaitConsumerThreadsFinished(std::vector<std::thread>* threads) {
    for (auto& thread : *threads) {
        thread.join();
    }
}

int ProcessError(const Error& err) {
    if (err == nullptr) return 0;
    std::cout << err->Explain() << std::endl;
    return err == asapo::ConsumerErrorTemplates::kEndOfStream ? 0 : 1;
}

BrokerPtr CreateBrokerAndGroup(const Args& args, Error* err) {
    auto broker = asapo::DataBrokerFactory::CreateServerBroker(args.server, args.file_path, true,
                  asapo::SourceCredentials{asapo::SourceType::kProcessed,args.beamtime_id, "", args.stream_in, args.token}, err);
    if (*err) {
        return nullptr;
    }

    broker->SetTimeout((uint64_t) args.timeout_ms);

    lock_in.lock();

    if (group_id.empty()) {
        group_id = broker->GenerateNewGroupId(err);
        if (*err) {
            lock_in.unlock();
            return nullptr;
        }
    }
    lock_in.unlock();
    return broker;
}

void GetBeamtimeMeta(const BrokerPtr& broker) {
    Error err;
    auto meta = broker->GetBeamtimeMeta(&err);
    if (err == nullptr) {
        std::cout << meta << std::endl;
    } else {
        std::cout << "Cannot get metadata: " << err->Explain() << std::endl;
    }
}

void SendDataDownstreamThePipeline(const Args& args, const asapo::FileInfo& fi, asapo::FileData data,
                                   const ProducerPtr& producer) {
    asapo::EventHeader header{fi.id, fi.size, fi.name, fi.metadata};
    Error err_send;
    if (args.transfer_data) {
        header.file_name += "_" + args.stream_out;
        err_send = producer->SendData(header, std::move(data), asapo::kDefaultIngestMode, ProcessAfterSend);
    } else {
        header.file_name = args.file_path + asapo::kPathSeparator + header.file_name;
        err_send = producer->SendData(header, nullptr, asapo::IngestModeFlags::kTransferMetaDataOnly, ProcessAfterSend);
        std::cout << err_send << std::endl;
    }

    lock_out.lock();
    if (!streamout_timer_started) {
        streamout_timer_started = true;
        streamout_start = high_resolution_clock::now();
    }
    lock_out.unlock();

    if (err_send) {
        std::cout << "Send error: " << err_send->Explain() << std::endl;
    }
}

Error ProcessNextEvent(const Args& args, const BrokerPtr& broker, const ProducerPtr& producer) {
    asapo::FileData data;
    asapo::FileInfo fi;

    auto err = broker->GetNext(&fi, group_id, args.transfer_data ? &data : nullptr);
    if (err) {
        return err;
    }

    SendDataDownstreamThePipeline(args, fi, std::move(data), producer);

    return nullptr;
}

std::vector<std::thread> StartConsumerThreads(const Args& args, const ProducerPtr& producer,
                                              std::vector<int>* nfiles,
                                              std::vector<int>* errors) {
    auto exec_next = [&args, nfiles, errors, &producer ](int i) {
        asapo::FileInfo fi;
        Error err;
        auto broker = CreateBrokerAndGroup(args, &err);
        if (err) {
            (*errors)[i] += ProcessError(err);
            return;
        }

        while (true) {
            auto err = ProcessNextEvent(args, broker, producer);
            if (err) {
                (*errors)[i] += ProcessError(err);
                if (err == asapo::ConsumerErrorTemplates::kEndOfStream || err == asapo::ConsumerErrorTemplates::kWrongInput) {
                    break;
                }
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

int ProcessAllData(const Args& args, const ProducerPtr& producer, uint64_t* duration_ms, int* nerrors) {
    asapo::FileInfo fi;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    std::vector<int> nfiles(args.nthreads, 0);
    std::vector<int> errors(args.nthreads, 0);

    auto threads = StartConsumerThreads(args, producer, &nfiles, &errors);
    WaitConsumerThreadsFinished(&threads);

    int n_total = std::accumulate(nfiles.begin(), nfiles.end(), 0);
    *nerrors = std::accumulate(errors.begin(), errors.end(), 0);

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration_read = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    *duration_ms = duration_read.count();
    return n_total;
}

std::unique_ptr<asapo::Producer> CreateProducer(const Args& args) {
    asapo::Error err;
    auto producer = asapo::Producer::Create(args.server, args.nthreads,
                                            asapo::RequestHandlerType::kTcp,
                                            asapo::SourceCredentials{asapo::SourceType::kProcessed,args.beamtime_id, "", args.stream_out, args.token }, 60, &err);
    if(err) {
        std::cerr << "Cannot start producer. ProducerError: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->EnableLocalLog(true);
    producer->SetLogLevel(asapo::LogLevel::Info);
    return producer;
}

int main(int argc, char* argv[]) {
    asapo::ExitAfterPrintVersionIfNeeded("GetNext Broker Example", argc, argv);
    Args args;
    if (argc != 11) {
        std::cout << "Usage: " + std::string{argv[0]}
                  + " <server> <files_path> <beamtime_id> <stream_in> <stream_out> <nthreads> <token> <timeout ms>  <timeout ms producer> <transfer data>"
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
    args.timeout_ms_producer = atoi(argv[9]);
    args.transfer_data = atoi(argv[10]) == 1;

    auto producer = CreateProducer(args);
    files_sent = 0;
    streamout_timer_started = false;

    uint64_t duration_ms;
    int nerrors;
    auto nfiles = ProcessAllData(args, producer, &duration_ms, &nerrors);

    if (producer->WaitRequestsFinished(args.timeout_ms_producer) != nullptr) {
        std::cerr << "Stream out exit on timeout " << std::endl;
    }
    auto duration_streamout = std::chrono::duration_cast<std::chrono::milliseconds>(streamout_finish - streamout_start);

    std::cout << "Stream in " << std::endl;
    std::cout << "  Processed " << nfiles << " file(s)" << std::endl;
    std::cout << "  Successfully: " << nfiles - nerrors << std::endl;
    std::cout << "  Errors : " << nerrors << std::endl;
    std::cout << "  Elapsed : " << duration_ms - args.timeout_ms << "ms" << std::endl;
    std::cout << "  Rate : " << 1000.0f * nfiles / (duration_ms - args.timeout_ms) << std::endl;

    std::cout << "Stream out " << std::endl;
    std::cout << "  Sent " << files_sent << " file(s)" << std::endl;
    std::cout << "  Elapsed : " << duration_streamout.count() << "ms" << std::endl;
    std::cout << "  Rate : " << 1000.0f * files_sent / (duration_streamout.count()) << std::endl;


    std::this_thread::sleep_for(std::chrono::milliseconds(1000));


    return (nerrors == 0) && (files_sent == nfiles) ? 0 : 1;
}
