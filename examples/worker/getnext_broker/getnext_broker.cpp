#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <iomanip>
#include <numeric>

#include "hidra2_worker.h"

using hidra2::WorkerErrorCode;
using std::chrono::high_resolution_clock;

void WaitThreads(std::vector<std::thread>* threads) {
    for (auto& thread : *threads) {
        thread.join();
    }
}

std::vector<std::thread> StartThreads(const std::string& server, const std::string& run_name, int nthreads,
                                      std::vector<int>* nfiles) {
    auto exec_next = [server, run_name, nfiles](int i) {
        hidra2::FileInfo fi;
        hidra2::WorkerErrorCode err;
        auto broker = hidra2::DataBrokerFactory::CreateServerBroker(server, run_name, &err);
        while (broker->GetNext(&fi, nullptr) == WorkerErrorCode::kOK) {
            (*nfiles)[i] ++;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < nthreads; i++) {
        threads.emplace_back(std::thread(exec_next, i));
    }
    return threads;
}

int ReadAllData(const std::string& server, const std::string& run_name, int nthreads, uint64_t* duration_ms) {
    hidra2::FileInfo fi;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    std::vector<int>nfiles(nthreads, 0);

    auto threads = StartThreads(server, run_name, nthreads, &nfiles);
    WaitThreads(&threads);

    int n_total = std::accumulate(nfiles.begin(), nfiles.end(), 0);

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
