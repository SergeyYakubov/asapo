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

std::vector<std::thread> StartThreads(const std::string& server, const std::string& run_name, int nthreads,
                                      std::vector<int>* nfiles, std::vector<int>* errors) {
    auto exec_next = [server, run_name, nfiles, errors](int i) {
        asapo::FileInfo fi;
        Error err;
        auto broker = asapo::DataBrokerFactory::CreateServerBroker(server, run_name, &err);
        broker->SetTimeout(10000);
        while ((err = broker->GetNext(&fi, nullptr)) == nullptr) {
            (*nfiles)[i] ++;
        }
        (*errors)[i] = ProcessError(err);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < nthreads; i++) {
        threads.emplace_back(std::thread(exec_next, i));
    }
    return threads;
}

int ReadAllData(const std::string& server, const std::string& run_name, int nthreads, uint64_t* duration_ms) {
    asapo::FileInfo fi;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    std::vector<int>nfiles(nthreads, 0);
    std::vector<int>errors(nthreads, 0);

    auto threads = StartThreads(server, run_name, nthreads, &nfiles, &errors);
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
    if (argc != 4) {
        std::cout << "Usage: " + std::string{argv[0]} +" <server> <run_name> <nthreads>" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string server = std::string{argv[1]};
    std::string run_name = std::string{argv[2]};
    int nthreads = atoi(argv[3]);


    uint64_t duration_ms;
    auto nfiles = ReadAllData(server, run_name, nthreads, &duration_ms);

    std::cout << "Processed " << nfiles << " file(s)" << std::endl;
    std::cout << "Elapsed : " << duration_ms << "ms" << std::endl;
    std::cout << "Rate : " << 1000.0f * nfiles / duration_ms << std::endl;
    return 0;
}
