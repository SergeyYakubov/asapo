#ifndef ASAPO_RDS_NET_SERVER_H
#define ASAPO_RDS_NET_SERVER_H

#include "../data_cache.h"
#include "common/error.h"
#include "receiver_data_server_request.h"

namespace asapo {

class RdsNetServer {
  public:
    /**
     * It is very important that this function is successfully called, before any other call is is made!
     */
    virtual Error Initialize() = 0;
    virtual GenericRequests GetNewRequests(Error* err) const noexcept = 0;
    virtual Error SendResponse(const ReceiverDataServerRequest* request,
                               const GenericNetworkResponse* response) const noexcept = 0;
    virtual Error
    SendResponseAndSlotData(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response,
                            const CacheMeta* cache_slot) const noexcept = 0;
    virtual void HandleAfterError(uint64_t source_id) const noexcept = 0;
    virtual ~RdsNetServer() = default;
};

}

#endif //ASAPO_RDS_NET_SERVER_H
