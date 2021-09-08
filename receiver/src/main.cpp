#include <iostream>
#include <utility>
#include "receiver.h"

#include "receiver_config_factory.h"
#include "receiver_config.h"

#include "receiver_data_server/receiver_data_server_logger.h"
#include "asapo/common/internal/version.h"

#include "receiver_data_server/receiver_data_server.h"
#include "receiver_data_server/net_server/rds_tcp_server.h"
#include "receiver_data_server/net_server/rds_fabric_server.h"
#include "monitoring/receiver_monitoring_client.h"

asapo::Error ReadConfigFile(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
        exit(EXIT_FAILURE);
    }
    asapo::ReceiverConfigFactory factory;
    return factory.SetConfig(argv[1]);
}

void AddDataServers(const asapo::ReceiverConfig* config, const asapo::SharedCache&,
                    const asapo::SharedReceiverMonitoringClient& monitoring,
                    std::vector<asapo::RdsNetServerPtr>& netServers) {
    auto logger = asapo::GetDefaultReceiverDataServerLogger();
    logger->SetLogLevel(config->log_level);

    auto ds_config = config->dataserver;
    auto networkingMode = ds_config.network_mode;
    if (std::find(networkingMode.begin(), networkingMode.end(), "tcp") != networkingMode.end()) {
        // Add TCP
        netServers.emplace_back(new asapo::RdsTcpServer("0.0.0.0:" + std::to_string(ds_config.listen_port), logger, monitoring));
    }

    if (std::find(networkingMode.begin(), networkingMode.end(), "fabric") != networkingMode.end()) {
        // Add Fabric
        netServers.emplace_back(new asapo::RdsFabricServer(ds_config.advertise_uri, logger, monitoring));
    }
}

asapo::SharedReceiverMonitoringClient StartMonitoringClient(const asapo::ReceiverConfig* config, asapo::SharedCache cache, asapo::Error* error) {
    bool useNoopImpl = !config->monitor_performance;
    auto monitoring = asapo::SharedReceiverMonitoringClient(asapo::GenerateDefaultReceiverMonitoringClient(cache, useNoopImpl));

    *error = nullptr;
    return monitoring;
}

std::vector<std::thread> StartDataServers(const asapo::ReceiverConfig* config, asapo::SharedCache cache,
                                          asapo::SharedReceiverMonitoringClient monitoring, asapo::Error* error) {
    std::vector<asapo::RdsNetServerPtr> netServers;
    std::vector<std::thread> dataServerThreads;

    AddDataServers(config, cache, monitoring, netServers);

    for (auto& server : netServers) {
        *error = server->Initialize();
        if (*error) {
            return {};
        }
    }

    dataServerThreads.reserve(netServers.size());
    for (auto& server : netServers) {
        // Allocate the server here in order to make sure all variables are still available
        auto data_server = new asapo::ReceiverDataServer{
            std::move(server),
            config->log_level,
            cache,
            config->dataserver};
        dataServerThreads.emplace_back(std::thread([data_server] {
            // We use a std::unique_ptr here in order to clean up the data_server once Run() is done.
            std::unique_ptr<asapo::ReceiverDataServer>(data_server)->Run();
        }));
    }

    return dataServerThreads;
}

int StartReceiver(const asapo::ReceiverConfig* config, asapo::SharedCache cache,
                  asapo::SharedReceiverMonitoringClient monitoring, asapo::AbstractLogger* logger) {
    static const std::string address = "0.0.0.0:" + std::to_string(config->listen_port);


    logger->Info(std::string("starting receiver, version ") + asapo::kVersion);
    auto* receiver = new asapo::Receiver(std::move(cache), std::move(monitoring));
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

    const auto& monitoringLogger = asapo::GetDefaultReceiverMonitoringLogger();
    monitoringLogger->SetLogLevel(config->log_level);

    asapo::SharedCache cache = nullptr;
    if (config->use_datacache) {
        cache.reset(new asapo::DataCache{config->datacache_size_gb * 1024 * 1024 * 1024, (float)config->datacache_reserved_share / 100});
    }

    auto monitoring = StartMonitoringClient(config, cache, &err);

    auto dataServerThreads = StartDataServers(config, cache, monitoring, &err);
    if (err) {
        logger->Error("Cannot start data server: " + err->Explain());
        return 1;
    }

    auto exit_code = StartReceiver(config, cache, monitoring, logger);
    return exit_code;
}
