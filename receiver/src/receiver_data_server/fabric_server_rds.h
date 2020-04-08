#ifndef ASAPO_RDS_FABRIC_SERVER_H
#define ASAPO_RDS_FABRIC_SERVER_H

#include "rds_net_server.h"
#include "asapo_fabric/asapo_fabric.h"

namespace asapo {

class FabricServerRds : public RdsNetServer {
public:
    explicit FabricServerRds(const std::string& address);
    ~FabricServerRds() override;
    std::unique_ptr<asapo::fabric::FabricFactory> factory__; // modified in testings to mock system calls, otherwise do not touch
public: // NetServer implementation
    GenericRequests GetNewRequests(Error* err) const noexcept override;

    Error SendResponse(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response) const noexcept override;

    Error SendResponseAndSlotData(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response,
                                  const CacheMeta* cache_slot) const noexcept override;

    void HandleAfterError(uint64_t source_id) const noexcept override;
};

}

#endif //ASAPO_RDS_FABRIC_SERVER_H
