#include <iostream>
#include <chrono>
#include <vector>
#include <mutex>
#include <thread>
#include <csignal>
#include <atomic>

#include "asapo/asapo_producer.h"
#include "eventmon_config.h"
#include "eventmon_config_factory.h"
#include "event_detector_factory.h"
#include "eventmon_logger.h"
#include "event_monitor_error.h"
#include "asapo/preprocessor/definitions.h"

#include "asapo/io/io_factory.h"
#include "asapo/common/version.h"

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
    auto producer = Producer::Create(config->asapo_endpoint, (uint8_t) config->nthreads,
                                     config->mode, asapo::SourceCredentials{asapo::SourceType::kProcessed,config->beamtime_id, "", config->stream, ""}, 3600, &err);
    if(err) {
        std::cerr << "cannot create producer: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->SetLogLevel(config->log_level);
    return producer;
}


void ProcessAfterSend(asapo::RequestCallbackPayload payload, asapo::Error err) {
    if (err) {
        const auto logger = asapo::GetDefaultEventMonLogger();
        logger->Error("data was not successfully send: " + err->Explain());
        return;
    }
    auto config = GetEventMonConfig();
    if (!config->remove_after_send) {
        return;
    }
    std::string fname = config->root_monitored_folder + asapo::kPathSeparator + payload.original_header.message;
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


void HandleSubsets(asapo::EventHeader* header) {
    switch (GetEventMonConfig()->subset_mode) {
    case asapo::SubSetMode::kNone:
        return;
    case asapo::SubSetMode::kBatch:
        header->subset_size = GetEventMonConfig()->subset_batch_size;
        header->id_in_subset = (header->file_id - 1) % header->subset_size + 1;
        header->file_id = (header->file_id - 1) / header->subset_size + 1;
        break;
    case asapo::SubSetMode::kMultiSource:
        header->subset_size = GetEventMonConfig()->subset_multisource_nsources;
        header->id_in_subset = GetEventMonConfig()->subset_multisource_sourceid;
        break;
    }
}

int main (int argc, char* argv[]) {
    asapo::ExitAfterPrintVersionIfNeeded("ASAPO Event Monitor", argc, argv);

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
    logger->Info(std::string("starting ASAPO Event Monitor, version ") + asapo::kVersion);
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
        event_header.file_id = ++i;
        HandleSubsets(&event_header);
        producer->SendFile(event_header, GetEventMonConfig()->root_monitored_folder + asapo::kPathSeparator +
                           event_header.file_name, asapo::kDefaultIngestMode, ProcessAfterSend);
    }

    logger->Info("Producer exit. Processed " + std::to_string(i) + " files");
    return EXIT_SUCCESS;
}

