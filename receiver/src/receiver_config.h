#ifndef HIDRA2_RECEIVER_CONFIG_H
#define HIDRA2_RECEIVER_CONFIG_H

#include "io/io.h"
#include "common/error.h"

namespace hidra2 {

struct ReceiverConfig {
    std::string monitor_db_uri;
    std::string monitor_db_name;
    uint64_t listen_port = 0;
    bool write_to_disk = false;
};

const ReceiverConfig* GetReceiverConfig();

}


#endif //HIDRA2_RECEIVER_CONFIG_H
