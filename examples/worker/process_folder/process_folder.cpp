#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <iomanip>

#include "worker/data_broker.h"

using hidra2::WorkerErrorCode;
using std::chrono::high_resolution_clock;

struct Statistics {
    std::chrono::milliseconds duration_scan;
    std::chrono::milliseconds duration_read;
    int nfiles;
    double size_gb;
    double bandwidth;
};
std::string ProcessCommandArguments(int argc, char* argv[]){
    if (argc != 2) {
        std::cout << "Usage: " + std::string{argv[0]} +" <path to folder>" << std::endl;
        abort();
    }
    return argv[1];
}

std::unique_ptr<hidra2::DataBroker> CreateBroker(const std::string& folder){
    hidra2::WorkerErrorCode err;
    auto broker = hidra2::DataBrokerFactory::Create(folder, &err);
    if (err != WorkerErrorCode::OK) {
        std::cout << "Cannot create broker" << std::endl;
        abort();
    }

    return broker;
}

void ConnectToBrocker(std::unique_ptr<hidra2::DataBroker>* broker,Statistics* statistics){
    hidra2::WorkerErrorCode err;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    err = (*broker)->Connect();
    if (err != WorkerErrorCode::OK) {
        std::cout << "Cannot connect to broker" << std::endl;
        abort();
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    statistics->duration_scan = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 );
}

void ReadAllData(std::unique_ptr<hidra2::DataBroker>* broker,Statistics* statistics){
    hidra2::WorkerErrorCode err;
    hidra2::FileInfo file_info;
    hidra2::FileData file_data;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    int nfiles = 0;
    uint64_t size = 0;
    while ((err = (*broker)->GetNext(&file_info, &file_data)) == WorkerErrorCode::OK) {
        nfiles++;
        size += file_info.size;
    }
    if (err!=WorkerErrorCode::NO_DATA){
        std::cout << "Read error" << std::endl;
        abort();
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    statistics->nfiles = nfiles;
    statistics->size_gb = double(size) / 1024 / 1024 / 1024;
    statistics->duration_read=std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 );
    statistics->bandwidth = statistics->size_gb/statistics->duration_read.count()*1000;
}


void PrintStatistics(const Statistics& statistics){
    std::cout << "Processed " << statistics.nfiles << " files" << std::endl;
    std::cout << "Total size: " << std::setprecision(2) << statistics.size_gb << "GB" << std::endl;
    std::cout << "Elapsed scan : " << statistics.duration_scan.count() << "ms" << std::endl;
    std::cout << "Elapsed read : " << statistics.duration_read.count() << "ms" << std::endl;
    std::cout << "Bandwidth: " << std::setprecision(2) << statistics.bandwidth << "GB/sec" << std::endl;
}


int main(int argc, char* argv[]) {
    std::string folder = ProcessCommandArguments(argc,argv);
    auto broker = CreateBroker(folder);

    Statistics statistics;
    ConnectToBrocker(&broker,&statistics);
    ReadAllData(&broker,&statistics);
    PrintStatistics(statistics);

    return 0;
}
