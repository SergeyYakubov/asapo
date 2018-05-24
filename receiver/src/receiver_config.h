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
    std::string broker_db_name;
    uint64_t listen_port = 0;
    bool write_to_disk = false;
    bool write_to_db = false;
    LogLevel log_level = LogLevel::Info;
    std::string tag;
};

const ReceiverConfig* GetReceiverConfig();

}


#endif //ASAPO_RECEIVER_CONFIG_H
