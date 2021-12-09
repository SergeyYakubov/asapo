#include <iostream>
#include <utility>
#include "receiver.h"

#include "receiver_config.h"

#include "receiver_data_server/receiver_data_server_logger.h"
#include "asapo/common/internal/version.h"

#include "receiver_data_server/receiver_data_server.h"
#include "receiver_data_server/net_server/rds_tcp_server.h"
#include "receiver_data_server/net_server/rds_fabric_server.h"
#include "monitoring/receiver_monitoring_client.h"

#include "metrics/receiver_prometheus_metrics.h"
#include "metrics/receiver_mongoose_server.h"

void ReadConfigFile(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config file>" << std::endl;
        exit(EXIT_FAILURE);
    }
    asapo::ReceiverConfigManager config_manager;
    auto err =  config_manager.ReadConfigFromFile(argv[1]);
    if (err) {
        std::cerr << "cannot read config file:" << err->Explain() << std::endl;
        exit(EXIT_FAILURE);
    }
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
    if (err) {
        logger->Error("failed to start receiver: " + err->Explain());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

std::unique_ptr<std::thread> StartMetricsServer(const asapo::ReceiverMetricsConfig& config,
                                                const asapo::AbstractLogger* logger) {
    if (!config.expose) {
        return nullptr;
    }
    return std::unique_ptr<std::thread> {
        new std::thread{
            [config, logger] {
                auto srv = std::unique_ptr<asapo::ReceiverMetricsServer>(new asapo::ReceiverMongooseServer());
                auto* provider = new asapo::ReceiverPrometheusMetrics();
                logger->Debug("metrics server listening on " + std::to_string(config.listen_port));
                srv->ListenAndServe(std::to_string(config.listen_port),
                                    std::unique_ptr<asapo::ReceiverMetricsProvider>(provider));
            }
        }
    };
}



int main(int argc, char* argv[]) {
    asapo::ExitAfterPrintVersionIfNeeded("ASAPO Receiver", argc, argv);
    ReadConfigFile(argc, argv);
    const auto& logger = asapo::GetDefaultReceiverLogger();
    auto config = asapo::GetReceiverConfig();
    logger->SetLogLevel(config->log_level);

    const auto& monitoringLogger = asapo::GetDefaultReceiverMonitoringLogger();
    monitoringLogger->SetLogLevel(config->log_level);

    asapo::SharedCache cache = nullptr;
    if (config->use_datacache) {
        cache.reset(new asapo::DataCache{config->datacache_size_gb * 1024 * 1024 * 1024,
                                         (float) config->datacache_reserved_share / 100});
    }

    asapo::Error err;
    auto monitoring = StartMonitoringClient(config, cache, &err);
    auto dataServerThreads = StartDataServers(config, cache, monitoring, &err);
    if (err) {
        logger->Error("cannot start data server: " + err->Explain());
        return EXIT_FAILURE;
    }

    auto metrics_thread = StartMetricsServer(config->metrics, logger);
    auto exit_code = StartReceiver(config, cache, monitoring, logger);
    // todo: implement graceful exit, currently it never reaches this point
    return exit_code;
}
