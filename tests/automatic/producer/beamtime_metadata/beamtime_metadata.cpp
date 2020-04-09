#include <iostream>

#include "asapo_producer.h"
#include <thread>


using std::chrono::system_clock;


struct Args {
    std::string discovery_service_endpoint;
    std::string beamtime_id;
    int mode;
};

void PrintCommandArguments(const Args& args) {
    std::cout << "discovery_service_endpoint: " << args.discovery_service_endpoint << std::endl
              << "beamtime_id: " << args.beamtime_id << std::endl
              << "mode: " << args.mode << std::endl
              << std::endl;
}

void ProcessCommandArguments(int argc, char* argv[], Args* args) {
    asapo::ExitAfterPrintVersionIfNeeded("dummy beamtime metadata", argc, argv);
    if (argc != 4) {
        std::cout <<
                  "Usage: " << argv[0] <<
                  " <destination> <beamtime_id>"
                  " <mode 0 -t tcp, 1 - filesystem>"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    try {
        args->discovery_service_endpoint = argv[1];
        args->beamtime_id = argv[2];
        args->mode = std::stoull(argv[3]);
        PrintCommandArguments(*args);
        return;
    } catch (std::exception& e) {
        std::cerr << "Fail to parse arguments" << std::endl;
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ProcessAfterSend(asapo::GenericRequestHeader header, asapo::Error err) {
    if (err) {
        std::cerr << "metadata was not successfully send: " << err << std::endl;
        return;
    } else {
        std::cout << "metadata was successfully send" << std::endl;
        return;
    }
}

bool SendMetaData(asapo::Producer* producer) {

    auto err = producer->SendMetaData("hello", &ProcessAfterSend);
    if (err) {
        std::cerr << "Cannot send metadata: " << err << std::endl;
        return false;
    }

    return true;
}

std::unique_ptr<asapo::Producer> CreateProducer(const Args& args) {
    asapo::Error err;
    auto producer = asapo::Producer::Create(args.discovery_service_endpoint, 1,
                                            args.mode == 0 ? asapo::RequestHandlerType::kTcp
                                            : asapo::RequestHandlerType::kFilesystem,
                                            asapo::SourceCredentials{args.beamtime_id, "", "", ""}, 60, &err);
    if (err) {
        std::cerr << "Cannot start producer. ProducerError: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->EnableLocalLog(true);
    producer->SetLogLevel(asapo::LogLevel::Info);
    return producer;
}

int main(int argc, char* argv[]) {
    Args args;
    ProcessCommandArguments(argc, argv, &args);

    auto producer = CreateProducer(args);
    SendMetaData(producer.get());

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    return EXIT_SUCCESS;
}

