#include <iostream>
#include "receiver.h"

#include "receiver_config_factory.h"
#include "receiver_config.h"

#include "receiver_logger.h"
#include "common/version.h"

#include "receiver_data_server/receiver_data_server.h"

#include "data_cache.h"

asapo::Error ReadConfigFile(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
        exit(EXIT_FAILURE);
    }
    asapo::ReceiverConfigFactory factory;
    return factory.SetConfig(argv[1]);
}

std::thread StartDataServer(const asapo::ReceiverConfig* config, asapo::SharedCache cache) {
    static const std::string dataserver_address = "0.0.0.0:" + std::to_string(config->dataserver_listen_port);
    return std::thread([config, cache] {
        asapo::ReceiverDataServer data_server{dataserver_address, config->log_level, (uint8_t)config->dataserver_nthreads, cache};
        data_server.Run();
    });
}

int StartReceiver(const asapo::ReceiverConfig* config, asapo::SharedCache cache,
                  asapo::AbstractLogger* logger) {
    static const std::string address = "0.0.0.0:" + std::to_string(config->listen_port);


    logger->Info(std::string("starting receiver, version ") + asapo::kVersion);
    auto* receiver = new asapo::Receiver(cache);
    logger->Info("listening on " + address);

    asapo::Error err;
    receiver->Listen(address, &err);
    if(err) {
        logger->Error("failed to start receiver: " + err->Explain());
        return 1;
    }
    return 0;
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

    asapo::SharedCache cache = nullptr;
    if (config->use_datacache) {
        cache.reset(new asapo::DataCache{config->datacache_size_gb * 1024 * 1024 * 1024, (float)config->datacache_reserved_share / 100});
    }

    auto data_thread = StartDataServer(config, cache);
    auto exit_code = StartReceiver(config, cache, logger);
    return exit_code;
}
