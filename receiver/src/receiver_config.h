#ifndef HIDRA2_RECEIVER_CONFIG_H
#define HIDRA2_RECEIVER_CONFIG_H

#include "io/io.h"
#include "common/error.h"

namespace hidra2 {

struct ReceiverConfig {
    std::string influxdb_uri;
};

const ReceiverConfig* GetReceiverConfig();

}


#endif //HIDRA2_RECEIVER_CONFIG_H
