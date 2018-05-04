#include <iostream>
#include "receiver.h"

#include "receiver_config_factory.h"
#include "receiver_config.h"

#include "receiver_logger.h"

hidra2::Error ReadConfigFile(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
        exit(EXIT_FAILURE);
    }
    hidra2::ReceiverConfigFactory factory;
    return factory.SetConfigFromFile(argv[1]);
}

int main (int argc, char* argv[]) {

    auto err = ReadConfigFile(argc, argv);
    const auto& logger = hidra2::GetDefaultReceiverLogger();

    if (err) {
        logger->Error("cannot read config file: " + err->Explain());
        return 1;
    }

    auto config = hidra2::GetReceiverConfig();

    logger->SetLogLevel(config->log_level);

    static const std::string address = "0.0.0.0:" + std::to_string(config->listen_port);

    auto* receiver = new hidra2::Receiver();

    logger->Info("listening on " + address);
    receiver->Listen(address, &err);
    if(err) {
        logger->Error("failed to start receiver: " + err->Explain());
        return 1;
    }
    return 0;
}
