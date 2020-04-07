#ifndef ASAPO_NET_SERVER_H
#define ASAPO_NET_SERVER_H

#include "common/error.h"

#include "request/request.h"
#include "../data_cache.h"

namespace asapo {

class NetServer {
  public:
    virtual GenericRequests GetNewRequests(Error* err) const noexcept = 0;
    virtual Error SendResponse(uint64_t source_id, GenericNetworkResponse* response) const noexcept = 0;
    virtual Error SendResponseAndSlotData(uint64_t source_id, GenericNetworkResponse* response,
                                          GenericRequestHeader* request, CacheMeta* cache_slot) const noexcept = 0;
    virtual void HandleAfterError(uint64_t source_id) const noexcept = 0;
    virtual ~NetServer() = default;
};

}

#endif //ASAPO_NET_SERVER_H
