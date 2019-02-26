#ifndef ASAPO_NET_SERVER_H
#define ASAPO_NET_SERVER_H

#include "common/error.h"

#include "request/request.h"

namespace asapo {

class NetServer {
  public:
    virtual GenericRequests GetNewRequests(Error* err) const noexcept = 0;
    virtual Error SendData(uint64_t source_id, void* buf, uint64_t size) const noexcept = 0;
    virtual void HandleAfterError(uint64_t source_id) const noexcept = 0;
    virtual ~NetServer() = default;
};

}

#endif //ASAPO_NET_SERVER_H
