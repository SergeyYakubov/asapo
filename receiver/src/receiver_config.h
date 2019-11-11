#ifndef ASAPO_RECEIVER_CONFIG_H
#define ASAPO_RECEIVER_CONFIG_H

#include "io/io.h"
#include "common/error.h"
#include "logger/logger.h"

#include "receiver_data_server/receiver_datacenter_config.h"
namespace asapo {

struct ReceiverConfig {
    std::string performance_db_uri;
    std::string performance_db_name;
    std::string database_uri;
    std::string root_folder;
    uint64_t listen_port = 0;
    std::string authorization_server;
    uint64_t authorization_interval_ms = 0;
    bool write_to_disk = false;
    bool write_to_db = false;
    bool use_datacache = true;
    uint64_t datacache_size_gb = 0;
    uint64_t datacache_reserved_share = 0;
    uint64_t receive_to_disk_threshold_mb = 0;
    LogLevel log_level = LogLevel::Info;
    std::string tag;
    std::string advertise_ip;
    ReceiverDataCenterConfig dataserver;
    std::string discovery_server;
};

const ReceiverConfig* GetReceiverConfig();

}


#endif //ASAPO_RECEIVER_CONFIG_H
