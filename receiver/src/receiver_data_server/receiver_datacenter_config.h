#ifndef ASAPO_RECEIVER_DATACENTER_CONFIG_H
#define ASAPO_RECEIVER_DATACENTER_CONFIG_H

#include <string>

namespace asapo {

struct ReceiverDataCenterConfig {
    uint64_t listen_port = 0;
    uint64_t nthreads = 0;
    std::string tag;
    std::string advertise_uri;
};

}


#endif //ASAPO_RECEIVER_DATACENTER_CONFIG_H
