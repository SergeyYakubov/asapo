#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <iomanip>
#include <numeric>

#include "asapo_worker.h"

using std::chrono::high_resolution_clock;
using asapo::Error;

struct Params {
    std::string server;
    std::string file_path;
    std::string beamtime_id;
    std::string token;
    int timeout_ms;
    int nthreads;
    bool read_data;
};

void WaitThreads(std::vector<std::thread>* threads) {
    for (auto& thread : *threads) {
        thread.join();
    }
}

int ProcessError(const Error& err) {
    if (err == nullptr) return 0;
    if (err->GetErrorType() != asapo::ErrorType::kTimeOut) {
        std::cout << err->Explain() << std::endl;
        return 1;
    }
    std::cout << err->Explain() << std::endl;
    return 0;
}

std::vector<std::thread> StartThreads(const Params& params, std::vector<int>* nfiles, std::vector<int>* errors) {
    auto exec_next = [&params, nfiles, errors](int i) {
        asapo::FileInfo fi;
        Error err;
        auto broker = asapo::DataBrokerFactory::CreateServerBroker(params.server, params.file_path, params.beamtime_id,
                      params.token, &err);
        broker->SetTimeout(params.timeout_ms);
        asapo::FileData data;
        while ((err = broker->GetNext(&fi, params.read_data ? &data : nullptr)) == nullptr) {
            if (params.read_data && (*nfiles)[i] < 10 && fi.size < 10) {
                data[9] = 0;
                std::cout << "Received: " << reinterpret_cast<char const*>(data.get()) << std::endl;
            }
            (*nfiles)[i] ++;
        }
        (*errors)[i] = ProcessError(err);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < params.nthreads; i++) {
        threads.emplace_back(std::thread(exec_next, i));
    }
    return threads;
}

int ReadAllData(const Params& params, uint64_t* duration_ms) {
    asapo::FileInfo fi;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    std::vector<int>nfiles(params.nthreads, 0);
    std::vector<int>errors(params.nthreads, 0);

    auto threads = StartThreads(params, &nfiles, &errors);
    WaitThreads(&threads);

    int n_total = std::accumulate(nfiles.begin(), nfiles.end(), 0);
    int errors_total = std::accumulate(errors.begin(), errors.end(), 0);

    if (errors_total) {
        exit(EXIT_FAILURE);
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration_read = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 );
    *duration_ms = duration_read.count();
    return n_total;
}

int main(int argc, char* argv[]) {
    asapo::ExitAfterPrintVersionIfNeeded("GetNext Broker Example", argc, argv);
    if (argc != 8) {
        std::cout << "Usage: " + std::string{argv[0]} +" <server> <files_path> <run_name> <nthreads> <token> <timeout ms> <metaonly>"
                  <<
                  std::endl;
        exit(EXIT_FAILURE);
    }
    Params params;
    params.server = std::string{argv[1]};
    params.file_path = std::string{argv[2]};
    params.beamtime_id = std::string{argv[3]};
    params.nthreads = atoi(argv[4]);
    params.token = std::string{argv[5]};
    params.timeout_ms = atoi(argv[6]);
    params.read_data = atoi(argv[7]) != 1;

    uint64_t duration_ms;
    auto nfiles = ReadAllData(params, &duration_ms);

    std::cout << "Processed " << nfiles << " file(s)" << std::endl;
    std::cout << "Elapsed : " << duration_ms << "ms" << std::endl;
    std::cout << "Rate : " << 1000.0f * nfiles / (duration_ms - 10000) << std::endl;
    return 0;
}
