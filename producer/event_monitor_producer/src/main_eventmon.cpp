#include <iostream>
#include <chrono>
#include <vector>
#include <mutex>
#include <thread>
#include <csignal>
#include <atomic>

#include "asapo_producer.h"
#include "eventmon_config.h"
#include "eventmon_config_factory.h"
#include "event_detector_factory.h"
#include "eventmon_logger.h"
#include "event_monitor_error.h"
#include "preprocessor/definitions.h"

#include "io/io_factory.h"

using asapo::Producer;
using asapo::EventMonConfigFactory;
using asapo::Error;
using asapo::GetEventMonConfig;

auto io = asapo::GenerateDefaultIO();

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
        const auto logger = asapo::GetDefaultEventMonLogger();
        logger->Error("data was not successfully send: " + err->Explain());
        return;
    }
    auto config = GetEventMonConfig();
    std::string fname = config->root_monitored_folder + asapo::kPathSeparator + header.message;
    auto error = io->RemoveFile(fname);
    if (error) {
        const auto logger = asapo::GetDefaultEventMonLogger();
        logger->Error("cannot delete file: " + fname + "" + error->Explain());
        return;
    }
}

volatile sig_atomic_t stop_signal;

void SignalHandler(int signal) {
    stop_signal = signal;
}


int main (int argc, char* argv[]) {
    auto err = ReadConfigFile(argc, argv);
    if (err) {
        std::cerr << "cannot read config file: " << err->Explain() << std::endl;
        return EXIT_FAILURE;
    }

    stop_signal = 0;
    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);
#if defined(__linux__) || defined (__APPLE__)
    siginterrupt(SIGINT, 1);
#endif

    const auto logger = asapo::GetDefaultEventMonLogger();
    logger->SetLogLevel(GetEventMonConfig()->log_level);


    auto producer = CreateProducer();
    asapo::EventDetectorFactory factory;
    auto event_detector = factory.CreateEventDetector();

    err = event_detector->StartMonitoring();
    if (err) {
        logger->Error(err->Explain());
        return EXIT_FAILURE;
    }

    int i = 0;
    while (true) {
        asapo::EventHeader event_header;
        auto err = event_detector->GetNextEvent(&event_header);
        if (stop_signal) {
            break; // we check it here because signal can interrupt system call (ready by inotify and result in incomplete event data)
        }
        if (err) {
            if (err != asapo::EventMonitorErrorTemplates::kNoNewEvent) {
                logger->Error("cannot retrieve next event: " + err->Explain());
            }
            continue;
        }
        event_header.file_id = i++;
        producer->SendFile(event_header, GetEventMonConfig()->root_monitored_folder + asapo::kPathSeparator +
                           event_header.file_name, ProcessAfterSend);
    }

    logger->Info("Producer exit. Processed " + std::to_string(i) + " files");
    return EXIT_SUCCESS;
}

