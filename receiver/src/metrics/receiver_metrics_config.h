#ifndef ASAPO_RECEIVER_SRC_METRICS_RECEIVER_METRICS_CONFIG_H_
#define ASAPO_RECEIVER_SRC_METRICS_RECEIVER_METRICS_CONFIG_H_

#include <string>

namespace asapo {

struct ReceiverMetricsConfig {
    bool expose = false;
    uint64_t listen_port = 0;
};

}

#endif //ASAPO_RECEIVER_SRC_METRICS_RECEIVER_METRICS_CONFIG_H_
