#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <mutex>

#include "asapo_consumer.h"

using std::chrono::system_clock;
using asapo::Error;

std::string group_id = "";
std::mutex lock;

struct Args {
    std::string server;
    std::string file_path;
    std::string beamtime_id;
    std::string token;
    int timeout_ms;
    int nthreads;
    bool read_data;
    bool datasets;
};

void WaitThreads(std::vector<std::thread>* threads) {
    for (auto& thread : *threads) {
        thread.join();
    }
}

int ProcessError(const Error& err) {
    if (err == nullptr) return 0;
    std::cout << err->Explain() << std::endl;
    return err == asapo::ConsumerErrorTemplates::kEndOfStream ? 0 : 1;
}

std::vector<std::thread> StartThreads(const Args& params,
                                      std::vector<int>* nfiles,
                                      std::vector<int>* errors,
                                      std::vector<int>* nbuf,
                                      std::vector<int>* nfiles_total) {
    auto exec_next = [&params, nfiles, errors, nbuf, nfiles_total](int i) {
        asapo::FileInfo fi;
        Error err;
        auto broker = asapo::DataBrokerFactory::CreateServerBroker(params.server, params.file_path,
                      asapo::SourceCredentials{params.beamtime_id, "", "", params.token}, &err);
        broker->SetTimeout((uint64_t) params.timeout_ms);
        asapo::FileData data;

        lock.lock();

        if (group_id.empty()) {
            group_id = broker->GenerateNewGroupId(&err);
            if (err) {
                (*errors)[i] += ProcessError(err);
                return;
            }
        }

        lock.unlock();

        auto start = system_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(system_clock::now() - start).count() <
                params.timeout_ms) {
            if (params.datasets) {
                auto dataset = broker->GetLastDataset(group_id, &err);
                if (err == nullptr) {
                    for (auto& fi : dataset.content) {
                        (*nbuf)[i] += fi.buf_id == 0 ? 0 : 1;
                        (*nfiles_total)[i]++;
                    }
                }
            } else {
                err = broker->GetLast(&fi, group_id, params.read_data ? &data : nullptr);
                if (err == nullptr) {
                    (*nbuf)[i] += fi.buf_id == 0 ? 0 : 1;
                    if (params.read_data && (*nfiles)[i] < 10 && fi.size < 10) {
                        data[9] = 0;
                        std::cout << "Received: " << reinterpret_cast<char const*>(data.get()) << std::endl;
                    }
                }
            }
            if (err) {
                (*errors)[i] += ProcessError(err);
                if (err == asapo::ConsumerErrorTemplates::kEndOfStream) {
                    break;
                }
            }
            (*nfiles)[i]++;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < params.nthreads; i++) {
        threads.emplace_back(std::thread(exec_next, i));
    }
    return threads;
}

int ReadAllData(const Args& params, uint64_t* duration_ms, int* nerrors, int* nbuf, int* nfiles_total) {
    asapo::FileInfo fi;
    system_clock::time_point t1 = system_clock::now();

    std::vector<int> nfiles(params.nthreads, 0);
    std::vector<int> errors(params.nthreads, 0);
    std::vector<int> nfiles_frombuf(params.nthreads, 0);
    std::vector<int> nfiles_total_in_datasets(params.nthreads, 0);

    auto threads = StartThreads(params, &nfiles, &errors, &nfiles_frombuf, &nfiles_total_in_datasets);
    WaitThreads(&threads);

    int n_total = std::accumulate(nfiles.begin(), nfiles.end(), 0);
    *nerrors = std::accumulate(errors.begin(), errors.end(), 0);
    *nbuf = std::accumulate(nfiles_frombuf.begin(), nfiles_frombuf.end(), 0);
    *nfiles_total = std::accumulate(nfiles_total_in_datasets.begin(), nfiles_total_in_datasets.end(), 0);

    system_clock::time_point t2 = system_clock::now();
    auto duration_read = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    *duration_ms = duration_read.count();
    return n_total;
}

int main(int argc, char* argv[]) {
    asapo::ExitAfterPrintVersionIfNeeded("GetLast Broker Example", argc, argv);
    Args params;
    params.datasets = false;
    if (argc != 8 && argc != 9) {
        std::cout << "Usage: " + std::string{argv[0]}
                  + " <server> <files_path> <run_name> <nthreads> <token> <timeout ms> <metaonly> [use datasets]"
                  <<
                  std::endl;
        exit(EXIT_FAILURE);
    }
    params.server = std::string{argv[1]};
    params.file_path = std::string{argv[2]};
    params.beamtime_id = std::string{argv[3]};
    params.nthreads = atoi(argv[4]);
    params.token = std::string{argv[5]};
    params.timeout_ms = atoi(argv[6]);
    params.read_data = atoi(argv[7]) != 1;
    if (argc == 9) {
        params.datasets = atoi(argv[8]) == 1;
    }
    uint64_t duration_ms;
    int nerrors, nbuf, nfiles_total;
    auto nfiles = ReadAllData(params, &duration_ms, &nerrors, &nbuf, &nfiles_total);
    std::cout << "Processed " << nfiles << (params.datasets ? " dataset(s)" : " file(s)") << std::endl;
    if (params.datasets) {
        std::cout << "  with " << nfiles_total << " file(s)" << std::endl;
    }
    std::cout << "Successfully: " << nfiles - nerrors << std::endl;
    if (params.read_data && !params.datasets) {
        std::cout << "  from memory buffer: " << nbuf << std::endl;
        std::cout << "  from filesystem: " << nfiles - nerrors - nbuf << std::endl;
    }
    std::cout << "Errors : " << nerrors << std::endl;
    std::cout << "Elapsed : " << duration_ms << "ms" << std::endl;
    std::cout << "Rate : " << 1000.0f * nfiles / (duration_ms) << " Hz" << std::endl;
    return nerrors == 0 ? 0 : 1;
}
