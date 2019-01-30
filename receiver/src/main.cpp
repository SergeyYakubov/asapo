#include <iostream>
#include "receiver.h"

#include "receiver_config_factory.h"
#include "receiver_config.h"

#include "receiver_logger.h"
#include "common/version.h"

#include "receiver_data_server/receiver_data_server.h"

asapo::Error ReadConfigFile(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
        exit(EXIT_FAILURE);
    }
    asapo::ReceiverConfigFactory factory;
    return factory.SetConfigFromFile(argv[1]);
}

int main (int argc, char* argv[]) {
    asapo::ExitAfterPrintVersionIfNeeded("ASAPO Receiver", argc, argv);

    auto err = ReadConfigFile(argc, argv);
    const auto& logger = asapo::GetDefaultReceiverLogger();
    if (err) {
        logger->Error("cannot read config file: " + err->Explain());
        return 1;
    }

    auto config = asapo::GetReceiverConfig();

    logger->SetLogLevel(config->log_level);

    static const std::string dataserver_address = "0.0.0.0:" + std::to_string(config->dataserver_listen_port);
    asapo::ReceiverDataServer data_server{dataserver_address, config->log_level};
    std::thread server_thread (&asapo::ReceiverDataServer::Run, &data_server);

    static const std::string address = "0.0.0.0:" + std::to_string(config->listen_port);

    auto* receiver = new asapo::Receiver();

    logger->Info(std::string("starting receiver, version ") + asapo::kVersion);
    logger->Info("listening on " + address);
    receiver->Listen(address, &err);
    if(err) {
        logger->Error("failed to start receiver: " + err->Explain());
        return 1;
    }
    return 0;
}
