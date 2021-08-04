#ifndef ASAPO_RDS_NET_SERVER_H
#define ASAPO_RDS_NET_SERVER_H

#include "../../data_cache.h"
#include "asapo/common/error.h"
#include "../receiver_data_server_request.h"
#include "../../monitoring/receiver_monitoring_client.h"

namespace asapo {

class RdsNetServer {
  public:
    /**
     * It is very important that this function is successfully called, before any other call is is made!
     */
    virtual Error Initialize() = 0;
    virtual GenericRequests GetNewRequests(Error* err) = 0;
    virtual Error SendResponse(const ReceiverDataServerRequest* request,
                               const GenericNetworkResponse* response) = 0;
    virtual Error
    SendResponseAndSlotData(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response,
                            const CacheMeta* cache_slot) = 0;
    virtual void HandleAfterError(uint64_t source_id) = 0;
    virtual ~RdsNetServer() = default;

    virtual SharedReceiverMonitoringClient Monitoring() = 0;
};

using RdsNetServerPtr = std::unique_ptr<asapo::RdsNetServer>;

}

#endif //ASAPO_RDS_NET_SERVER_H
