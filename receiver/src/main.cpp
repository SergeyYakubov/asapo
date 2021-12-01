#include <iostream>
#include <utility>
#include "receiver.h"

#include "receiver_config.h"

#include "receiver_data_server/receiver_data_server_logger.h"
#include "asapo/common/internal/version.h"
#include "asapo/kafka_client/kafka_client.h"

#include "receiver_data_server/receiver_data_server.h"
#include "receiver_data_server/net_server/rds_tcp_server.h"
#include "receiver_data_server/net_server/rds_fabric_server.h"

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

void AddDataServers(const asapo::ReceiverConfig* config, asapo::SharedCache,
                    std::vector<asapo::RdsNetServerPtr>& netServers) {
    auto logger = asapo::GetDefaultReceiverDataServerLogger();
    logger->SetLogLevel(config->log_level);

    auto ds_config = config->dataserver;
    auto networkingMode = ds_config.network_mode;
    if (std::find(networkingMode.begin(), networkingMode.end(), "tcp") != networkingMode.end()) {
        // Add TCP
        netServers.emplace_back(new asapo::RdsTcpServer("0.0.0.0:" + std::to_string(ds_config.listen_port), logger));
    }

    if (std::find(networkingMode.begin(), networkingMode.end(), "fabric") != networkingMode.end()) {
        // Add Fabric
        netServers.emplace_back(new asapo::RdsFabricServer(ds_config.advertise_uri, logger));
    }
}

std::vector<std::thread> StartDataServers(const asapo::ReceiverConfig* config, asapo::SharedCache cache,
                                          asapo::Error* error) {
    std::vector<asapo::RdsNetServerPtr> netServers;
    std::vector<std::thread> dataServerThreads;

    AddDataServers(config, cache, netServers);

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
                  asapo::KafkaClient* kafkaClient, asapo::AbstractLogger* logger) {
    static const std::string address = "0.0.0.0:" + std::to_string(config->listen_port);

    logger->Info(std::string("starting receiver, version ") + asapo::kVersion);
    auto* receiver = new asapo::Receiver(cache, kafkaClient);
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

    asapo::SharedCache cache = nullptr;
    asapo::KafkaClient* kafkaClient = nullptr;
    if (config->use_datacache) {
        cache.reset(new asapo::DataCache{config->datacache_size_gb * 1024 * 1024 * 1024,
                                         (float) config->datacache_reserved_share / 100});
    }

    asapo::Error err;
    auto dataServerThreads = StartDataServers(config, cache, &err);
    if (err) {
        logger->Error("cannot start data server: " + err->Explain());
        return EXIT_FAILURE;
    }

    auto metrics_thread = StartMetricsServer(config->metrics, logger);

    if (!config->kafka_config.global_config.empty()) {
        kafkaClient = asapo::CreateKafkaClient(config->kafka_config, &err);
        if (kafkaClient == nullptr) {
            logger->Error("Error initializing kafka client: " + err->Explain());
            return EXIT_FAILURE;
        }
    }
    else {
        logger->Info("Kafka notification disabled.");
    }

    auto exit_code = StartReceiver(config, cache, kafkaClient, logger);
// todo: implement graceful exit, currently it never reaches this point
    return exit_code;
}
