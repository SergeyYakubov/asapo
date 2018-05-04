#ifndef HIDRA2_RECEIVER_CONFIG_H
#define HIDRA2_RECEIVER_CONFIG_H

#include "io/io.h"
#include "common/error.h"
#include "logger/logger.h"

namespace hidra2 {

struct ReceiverConfig {
    std::string monitor_db_uri;
    std::string monitor_db_name;
    std::string broker_db_uri;
    std::string broker_db_name;
    uint64_t listen_port = 0;
    bool write_to_disk = false;
    bool write_to_db = false;
    LogLevel log_level = LogLevel::Info;
};

const ReceiverConfig* GetReceiverConfig();

}


#endif //HIDRA2_RECEIVER_CONFIG_H
