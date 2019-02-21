#ifndef ASAPO_RECEIVER_CONFIG_H
#define ASAPO_RECEIVER_CONFIG_H

#include "io/io.h"
#include "common/error.h"
#include "logger/logger.h"

namespace asapo {

struct ReceiverConfig {
    std::string monitor_db_uri;
    std::string monitor_db_name;
    std::string broker_db_uri;
    std::string root_folder;
    uint64_t listen_port = 0;
    uint64_t dataserver_listen_port = 0;
    std::string authorization_server;
    uint64_t authorization_interval_ms = 0;
    bool write_to_disk = false;
    bool write_to_db = false;
    bool use_datacache = true;
    uint64_t datacache_size_gb = 0;
    uint64_t datacache_reserved_share = 0;
    uint64_t dataserver_nthreads = 1;
    LogLevel log_level = LogLevel::Info;
    std::string tag;
    std::string source_host;
};

const ReceiverConfig* GetReceiverConfig();

}


#endif //ASAPO_RECEIVER_CONFIG_H
