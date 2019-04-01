#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <iomanip>

#include "asapo_worker.h"


using std::chrono::high_resolution_clock;
using asapo::Error;

struct Statistics {
    std::chrono::milliseconds duration_scan;
    std::chrono::milliseconds duration_read;
    int nfiles;
    double size_gb;
    double bandwidth;
};
std::string ProcessCommandArguments(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " + std::string{argv[0]} +" <path to folder>" << std::endl;
        exit(EXIT_FAILURE);
    }
    return argv[1];
}

std::unique_ptr<asapo::DataBroker> CreateBroker(const std::string& folder) {
    Error err;
    auto broker = asapo::DataBrokerFactory::CreateFolderBroker(folder, &err);
    if (err != nullptr) {
        std::cout << "Cannot create broker" << std::endl;
        exit(EXIT_FAILURE);
    }

    return broker;
}

void ConnectToBrocker(std::unique_ptr<asapo::DataBroker>* broker, Statistics* statistics) {
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    Error err = (*broker)->Connect();
    if (err != nullptr) {
        std::cout << err->Explain() << std::endl;
        exit(EXIT_FAILURE);
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    statistics->duration_scan = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 );
}

void ReadAllData(std::unique_ptr<asapo::DataBroker>* broker, Statistics* statistics) {
    Error err;
    asapo::FileInfo file_info;
    asapo::FileData file_data;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    int nfiles = 0;
    uint64_t size = 0;
    while ((err = (*broker)->GetNext(&file_info, "", &file_data)) == nullptr) {
        nfiles++;
        size += file_info.size;
    }
    if (err->GetErrorType() != asapo::ErrorType::kEndOfFile) {
        std::cout << err->Explain() << std::endl;
        exit(EXIT_FAILURE);
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    statistics->nfiles = nfiles;
    statistics->size_gb = double(size) / 1000 / 1000 / 1000;
    statistics->duration_read = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 );
    statistics->bandwidth = statistics->size_gb / statistics->duration_read.count() * 1000;
}


void PrintStatistics(const Statistics& statistics) {
    std::cout << "Processed " << statistics.nfiles << " file(s)" << std::endl;
    std::cout << "Total size: " << std::setprecision(2) << statistics.size_gb << "GB" << std::endl;
    std::cout << "Elapsed scan : " << statistics.duration_scan.count() << "ms" << std::endl;
    std::cout << "Elapsed read : " << statistics.duration_read.count() << "ms" << std::endl;
    std::cout << "Bandwidth: " << std::setprecision(2) << statistics.bandwidth << "GB/sec" << std::endl;
}


int main(int argc, char* argv[]) {
    asapo::ExitAfterPrintVersionIfNeeded("Process Folder Broker Example", argc, argv);

    std::string folder = ProcessCommandArguments(argc, argv);
    auto broker = CreateBroker(folder);

    Statistics statistics;
    ConnectToBrocker(&broker, &statistics);
    ReadAllData(&broker, &statistics);
    PrintStatistics(statistics);

    return 0;
}
