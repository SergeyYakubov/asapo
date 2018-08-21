#include <iostream>
#include <chrono>
#include <vector>
#include <mutex>
#include <thread>

#include "asapo_producer.h"
#include "foldermon_config.h"
#include "foldermon_config_factory.h"


using asapo::Producer;
using asapo::FolderMonConfigFactory;
using asapo::Error;
using asapo::GetFolderMonConfig;

Error ReadConfigFile(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
        exit(EXIT_FAILURE);
    }
    FolderMonConfigFactory factory;
    return factory.SetConfigFromFile(argv[1]);
}

std::unique_ptr<Producer> CreateProducer() {
    auto config = GetFolderMonConfig();


    Error err;
    auto producer = Producer::Create(config->asapo_endpoint, config->nthreads,
                                            config->mode, config->beamtime_id, &err);
    if(err) {
        std::cerr << "cannot create producer: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->SetLogLevel(config->log_level);
    return producer;
}


int main (int argc, char* argv[]) {
    auto err = ReadConfigFile(argc, argv);
    if (err) {
        std::cerr << "cannot read config file: " << err->Explain() << std::endl;
        return EXIT_FAILURE;
    }

    auto producer = CreateProducer();

    return EXIT_SUCCESS;
}

