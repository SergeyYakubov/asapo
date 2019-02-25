#ifndef ASAPO_RECEIVER_DATA_SERVER_REQUEST_H
#define ASAPO_RECEIVER_DATA_SERVER_REQUEST_H

#include "common/networking.h"

#include "request/request.h"

namespace asapo {

class NetServer;

class ReceiverDataServerRequest : public GenericRequest {
  public:
    explicit ReceiverDataServerRequest(GenericRequestHeader header, uint64_t source_id);
    const uint64_t source_id;
    ~ReceiverDataServerRequest() = default;

};

using ReceiverDataServerRequestPtr = std::unique_ptr<ReceiverDataServerRequest>;

}

#endif //ASAPO_RECEIVER_DATA_SERVER_REQUEST_H
