#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>


#include "worker/data_broker.h"

using hidra2::WorkerErrorCode;
using std::chrono::high_resolution_clock;


int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cout << "Usage: " + std::string{argv[0]} +" <path to folder>" << std::endl;
        abort();
    }

    std::string folder{argv[1]};

    hidra2::WorkerErrorCode err;
    auto broker = hidra2::DataBrokerFactory::Create(folder, &err);
    if (err != WorkerErrorCode::OK) {
        std::cout << "Cannot create broker" << std::endl;
        abort();
    }

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    err = broker->Connect();
    if (err != WorkerErrorCode::OK) {
        std::cout << "Cannot connect to broker" << std::endl;
        abort();
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration_scan = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();


    hidra2::FileInfo file_info;
    hidra2::FileData file_data;

    int nfiles = 0;
    uint64_t size = 0;
    while ((err = broker->GetNext(&file_info, &file_data)) == WorkerErrorCode::OK) {
        nfiles++;
        size += file_info.size;
    }
    high_resolution_clock::time_point t3 = high_resolution_clock::now();
    auto duration_read = std::chrono::duration_cast<std::chrono::milliseconds>( t3 - t2 ).count();

    std::cout << "Processed " << nfiles << " files" << std::endl;
    std::cout << "Total size: " << size / 1024 / 1024 / 1024 << "GB" << std::endl;
    std::cout << "Elapsed scan : " << duration_scan << "ms" << std::endl;
    std::cout << "Elapsed read : " << duration_read << "ms" << std::endl;

    return 0;
}
