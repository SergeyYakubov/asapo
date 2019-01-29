#ifndef ASAPO_REQUEST_H
#define ASAPO_REQUEST_H

#include "common/networking.h"

namespace asapo {

class NetServer;

struct Request {
    explicit Request(uint64_t net_id, const NetServer* server);
    GenericRequestHeader header;
    const uint64_t net_id;
    const NetServer* server;
};

}

#endif //ASAPO_REQUEST_H
