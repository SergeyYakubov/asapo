#ifndef ASAPO_RECEIVER_CONFIG_H
#define ASAPO_RECEIVER_CONFIG_H

#include "asapo/io/io.h"
#include "asapo/common/error.h"
#include "asapo/logger/logger.h"

#include "receiver_data_server/receiver_data_server_config.h"
#include "metrics/receiver_metrics_config.h"

namespace asapo {

struct ReceiverConfig {
    std::string performance_db_uri;
    std::string performance_db_name;
    std::string monitoring_server_url;
    bool monitor_performance = false;
    std::string database_uri;
    uint64_t listen_port = 0;
    std::string authorization_server;
    uint64_t authorization_interval_ms = 0;
    bool use_datacache = true;
    uint64_t datacache_size_gb = 0;
    uint64_t datacache_reserved_share = 0;
    uint64_t receive_to_disk_threshold_mb = 0;
    LogLevel log_level = LogLevel::Info;
    std::string tag;
    ReceiverDataServerConfig dataserver;
    ReceiverMetricsConfig metrics;
    std::string discovery_server;
};

class ReceiverConfigManager {
  public:
    ReceiverConfigManager();
    Error ReadConfigFromFile(std::string file_name);
  public:
    std::unique_ptr<IO> io__;
};


const ReceiverConfig* GetReceiverConfig();

}


#endif //ASAPO_RECEIVER_CONFIG_H
