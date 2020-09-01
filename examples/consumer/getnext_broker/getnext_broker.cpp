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

using std::chrono::system_clock;
using asapo::Error;

std::string group_id = "";
std::mutex lock;

uint64_t file_size = 0;

inline std::string ConnectionTypeToString(asapo::NetworkConnectionType type) {
    switch (type) {
    case asapo::NetworkConnectionType::kUndefined:
        return "No connection";
    case asapo::NetworkConnectionType::kAsapoTcp:
        return "TCP";
    case asapo::NetworkConnectionType::kFabric:
        return "Fabric";
    }
    return "Unknown type";
}

struct Args {
    std::string server;
    std::string network_type;
    std::string file_path;
    std::string beamtime_id;
    std::string stream;
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
                                      std::vector<int>* nfiles_total,
                                      std::vector<asapo::NetworkConnectionType>* connection_type) {
    auto exec_next = [&params, nfiles, errors, nbuf, nfiles_total, connection_type](int i) {
        asapo::FileInfo fi;
        Error err;
        auto broker = asapo::DataBrokerFactory::CreateServerBroker(params.server, params.file_path, true,
                      asapo::SourceCredentials{params.beamtime_id, "", params.stream, params.token}, params.network_type, &err);
        if (err) {
            std::cout << "Error CreateServerBroker: " << err << std::endl;
            exit(EXIT_FAILURE);
        }

        broker->SetTimeout((uint64_t) params.timeout_ms);
        asapo::FileData data;

        lock.lock();
        if (group_id.empty()) {
            group_id = broker->GenerateNewGroupId(&err);
            if (err) {
                (*errors)[i] += ProcessError(err);
                lock.unlock();
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
            if (params.datasets) {
                auto dataset = broker->GetNextDataset(group_id, &err);
                if (err == nullptr) {
                    for (auto& fi : dataset.content) {
                        (*nbuf)[i] += fi.buf_id == 0 ? 0 : 1;
                        (*nfiles_total)[i]++;
                    }
                }
            } else {
                err = broker->GetNext(&fi, group_id, params.read_data ? &data : nullptr);
                if (err == nullptr) {
                    (*nbuf)[i] += fi.buf_id == 0 ? 0 : 1;
                    if (file_size == 0) {
                        lock.lock();
                        file_size = fi.size;
                        lock.unlock();
                    }
                    if (params.read_data && (*nfiles)[i] < 10 && fi.size < 10) {
                        data[9] = 0;
                        std::cout << "Received: " << reinterpret_cast<char const*>(data.get()) << std::endl;
                    }
                }
            }

            if (err) {
                (*errors)[i] += ProcessError(err);
                if (err) {
                    std::cout << "Thread exit: " << i << std::endl;
                    break;
                }
            }
            (*nfiles)[i]++;
        }

        (*connection_type)[i] = broker->CurrentConnectionType();
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < params.nthreads; i++) {
        threads.emplace_back(std::thread(exec_next, i));
    }
    return threads;
}

int ReadAllData(const Args& params, uint64_t* duration_ms, int* nerrors, int* nbuf, int* nfiles_total,
                asapo::NetworkConnectionType* connectionType) {
    asapo::FileInfo fi;
    system_clock::time_point t1 = system_clock::now();

    std::vector<int> nfiles(params.nthreads, 0);
    std::vector<int> errors(params.nthreads, 0);
    std::vector<int> nfiles_frombuf(params.nthreads, 0);
    std::vector<int> nfiles_total_in_datasets(params.nthreads, 0);
    std::vector<asapo::NetworkConnectionType> connection_types(params.nthreads, asapo::NetworkConnectionType::kUndefined);

    auto threads = StartThreads(params, &nfiles, &errors, &nfiles_frombuf, &nfiles_total_in_datasets, &connection_types);
    WaitThreads(&threads);

    int n_total = std::accumulate(nfiles.begin(), nfiles.end(), 0);
    *nerrors = std::accumulate(errors.begin(), errors.end(), 0);
    *nbuf = std::accumulate(nfiles_frombuf.begin(), nfiles_frombuf.end(), 0);
    *nfiles_total = std::accumulate(nfiles_total_in_datasets.begin(), nfiles_total_in_datasets.end(), 0);

    system_clock::time_point t2 = system_clock::now();
    auto duration_read = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    *duration_ms = duration_read.count();

    // The following two loops will check if all threads that processed some data were using the same network type
    {
        int firstThreadThatActuallyProcessedData = 0;
        for (int i = 0; i < params.nthreads; i++) {
            if (nfiles[i] > 0) {
                firstThreadThatActuallyProcessedData = i;
                break;
            }
        }

        *connectionType = connection_types[firstThreadThatActuallyProcessedData];
        for (int i = 0; i < params.nthreads; i++) {
            if (*connectionType != connection_types[i] && nfiles[i] > 0) {
                // The output will look like this:
                // ERROR thread[0](processed 5 files) connection type is 'No connection' but thread[1](processed 3 files) is 'TCP'

                std::cout << "ERROR thread[" << i << "](processed " << nfiles[i] << " files) connection type is '" <<
                          ConnectionTypeToString(connection_types[i]) << "' but thread["
                          << firstThreadThatActuallyProcessedData << "](processed "
                          << nfiles[firstThreadThatActuallyProcessedData] << " files) is '" << ConnectionTypeToString(
                              *connectionType) << "'" << std::endl;
            }
        }
    }

    return n_total;
}

void TryGetStream(Args* args) {
    std::stringstream test(args->beamtime_id);
    std::string segment;
    std::vector<std::string> seglist;

    while(std::getline(test, segment, '%')) {
        seglist.push_back(segment);
    }
    if (seglist.size() == 1) {
        return;
    }
    if (seglist.size() > 1) {
        args->beamtime_id = seglist[0];
        args->stream = seglist[1];
    }
    return;

}


int main(int argc, char* argv[]) {
    asapo::ExitAfterPrintVersionIfNeeded("GetNext Broker Example", argc, argv);
    Args params;
    params.datasets = false;
    if (argc != 9 && argc != 10) {
        std::cout << "Usage: " + std::string{argv[0]}
                  + " <server> <network_type> <files_path> <run_name> <nthreads> <token> <timeout ms> <metaonly> [use datasets]"
                  <<
                  std::endl;
        exit(EXIT_FAILURE);
    }
    params.server = std::string{argv[1]};
    params.network_type = std::string{argv[2]};
    params.file_path = std::string{argv[3]};
    params.beamtime_id = std::string{argv[4]};
    TryGetStream(&params);
    params.nthreads = atoi(argv[5]);
    params.token = std::string{argv[6]};
    params.timeout_ms = atoi(argv[7]);
    params.read_data = atoi(argv[8]) != 1;
    if (argc == 10) {
        params.datasets = atoi(argv[9]) == 1;
    }
    uint64_t duration_ms;
    int nerrors, nbuf, nfiles_total;
    asapo::NetworkConnectionType connectionType;
    auto nfiles = ReadAllData(params, &duration_ms, &nerrors, &nbuf, &nfiles_total, &connectionType);
    std::cout << "Processed " << nfiles << (params.datasets ? " dataset(s)" : " file(s)") << std::endl;
    if (params.datasets) {
        std::cout << "  with " << nfiles_total << " file(s)" << std::endl;
    }
    std::cout << "Successfully: " << nfiles - nerrors << std::endl;
    if (params.read_data) {
        std::cout << "  from memory buffer: " << nbuf << std::endl;
        std::cout << "  from filesystem: " << nfiles - nerrors - nbuf << std::endl;
    }
    std::cout << "Errors : " << nerrors << std::endl;
    std::cout << "Elapsed : " << duration_ms << "ms" << std::endl;
    auto rate = 1000.0f * nfiles / (duration_ms - params.timeout_ms);
    auto bw_gbytes = rate * file_size / 1000.0f / 1000.0f / 1000.0f;
    std::cout << "Rate : " << rate << std::endl;
    if (file_size > 0) {
        std::cout << "Bandwidth " << bw_gbytes * 8 << " Gbit/s" << std::endl;
        std::cout << "Bandwidth " << bw_gbytes << " GBytes/s" << std::endl;
    }

    std::cout << "Using connection type: " << ConnectionTypeToString(connectionType) << std::endl;
    return nerrors == 0 ? 0 : 1;
}
