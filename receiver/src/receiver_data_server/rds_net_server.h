#ifndef ASAPO_RDS_NET_SERVER_H
#define ASAPO_RDS_NET_SERVER_H

#include "../data_cache.h"
#include "common/error.h"
#include "receiver_data_server_request.h"

namespace asapo {

class RdsNetServer {
  public:
    virtual GenericRequests GetNewRequests(Error* err) const noexcept = 0;
    virtual Error SendResponse(uint64_t source_id, const GenericNetworkResponse* response) const noexcept = 0;
    virtual Error
    SendResponseAndSlotData(const ReceiverDataServerRequest* request, uint64_t source_id,
                            const GenericNetworkResponse* response,
                            const CacheMeta* cache_slot) const noexcept = 0;
    virtual void HandleAfterError(uint64_t source_id) const noexcept = 0;
    virtual ~RdsNetServer() = default;
};

}

#endif //ASAPO_RDS_NET_SERVER_H
