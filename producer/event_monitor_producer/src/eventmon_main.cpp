#include <iostream>
#include <chrono>
#include <vector>
#include <mutex>
#include <thread>

#include "asapo_producer.h"
#include "eventmon_config.h"
#include "eventmon_config_factory.h"
#include "event_detector_factory.h"
#include "eventmon_logger.h"

using asapo::Producer;
using asapo::EventMonConfigFactory;
using asapo::Error;
using asapo::GetEventMonConfig;

Error ReadConfigFile(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
        exit(EXIT_FAILURE);
    }
    EventMonConfigFactory factory;
    return factory.SetConfigFromFile(argv[1]);
}

std::unique_ptr<Producer> CreateProducer() {
    auto config = GetEventMonConfig();


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


void ProcessAfterSend(asapo::GenericRequestHeader header, asapo::Error err) {
    if (err) {
        const auto& logger = asapo::GetDefaultEventMonLogger();
        logger->Error("data was not successfully send: " + err->Explain());
        return;
    }
}


int main (int argc, char* argv[]) {
    auto err = ReadConfigFile(argc, argv);
    if (err) {
        std::cerr << "cannot read config file: " << err->Explain() << std::endl;
        return EXIT_FAILURE;
    }

    const auto& logger = asapo::GetDefaultEventMonLogger();
    logger->SetLogLevel(GetEventMonConfig()->log_level);


    auto producer = CreateProducer();
    asapo::EventDetectorFactory factory;
    auto event_detector = factory.CreateEventDetector();

    int i = 0;
    while (true && i < 1000) {
        asapo::EventHeader event_header;
        event_header.file_id = i++;
        event_header.file_size = 0;
        event_header.file_name = std::to_string(i);
        auto err = event_detector->GetNextEvent(&event_header);
        if (err) {
            logger->Error("cannot retrieve next event: " + err->Explain());
            continue;
        }
        producer->Send(event_header, nullptr, ProcessAfterSend);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));



    return EXIT_SUCCESS;
}

