#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <iomanip>

#include "hidra2_worker.h"

using hidra2::WorkerErrorCode;
using std::chrono::high_resolution_clock;

int ReadAllData(const std::unique_ptr<hidra2::DataBroker>& broker, uint64_t* duration_ms) {
    int nfiles = 0;
    hidra2::FileInfo fi;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    while (broker->GetNext(&fi, nullptr) == WorkerErrorCode::kOK) {
        nfiles ++;
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration_read = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 );
    *duration_ms = duration_read.count();
    return nfiles;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " + std::string{argv[0]} +" <server> <run_name>" << std::endl;
        exit(EXIT_FAILURE);
    }
    hidra2::WorkerErrorCode err;
    std::string server = std::string{argv[1]};
    std::string run_name = std::string{argv[2]};

    auto broker = hidra2::DataBrokerFactory::CreateServerBroker(server, run_name, &err);

    uint64_t duration_ms;
    auto nfiles = ReadAllData(broker, &duration_ms);

    std::cout << "Processed " << nfiles << " file(s)" << std::endl;
    std::cout << "Elapsed : " << duration_ms << "ms" << std::endl;
    return 0;
}
