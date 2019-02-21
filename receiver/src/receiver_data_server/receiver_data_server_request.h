#ifndef ASAPO_REQUEST_H
#define ASAPO_REQUEST_H

#include "common/networking.h"

#include "request/request.h"

namespace asapo {

class NetServer;

class ReceiverDataServerRequest : public GenericRequest {
  public:
    explicit ReceiverDataServerRequest(GenericRequestHeader header, uint64_t net_id, const NetServer* server);
    const uint64_t net_id;
    const NetServer* server;
    ~ReceiverDataServerRequest() = default;

};

using ReceiverDataServerRequestPtr = std::unique_ptr<ReceiverDataServerRequest>;

}

#endif //ASAPO_REQUEST_H
