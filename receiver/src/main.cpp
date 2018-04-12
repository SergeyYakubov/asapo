#include <iostream>
#include "receiver.h"

#include "receiver_config_factory.h"
#include "receiver_config.h"


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
    if (err) {
        std::cerr << "Cannot read config file: " << err << std::endl;
        return 1;
    }

    auto config = hidra2::GetReceiverConfig();

    static const std::string address = "0.0.0.0:" + std::to_string(config->listen_port);

    auto* receiver = new hidra2::Receiver();

    std::cout << "Listening on " << address << std::endl;
    receiver->Listen(address, &err);
    if(err) {
        std::cerr << "Failed to start receiver: " << err << std::endl;
        return 1;
    }
    return 0;
}
