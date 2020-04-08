#ifndef ASAPO_RDS_FABRIC_SERVER_H
#define ASAPO_RDS_FABRIC_SERVER_H

#include "rds_net_server.h"
#include "asapo_fabric/asapo_fabric.h"

namespace asapo {

class FabricServerRds : public RdsNetServer {
  public:
    explicit FabricServerRds(std::string  listenAddress);
    ~FabricServerRds() override;

    // modified in testings to mock system calls, otherwise do not touch
    std::unique_ptr<asapo::fabric::FabricFactory> factory__;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
  private:
    std::string listenAddress_;
    std::unique_ptr<fabric::FabricServer> server_;
  public: // NetServer implementation
    Error Initialize() override;

    GenericRequests GetNewRequests(Error* err) override;

    Error SendResponse(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response) override;

    Error SendResponseAndSlotData(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response,
                                  const CacheMeta* cache_slot) override;

    void HandleAfterError(uint64_t source_id) override;
};

}

#endif //ASAPO_RDS_FABRIC_SERVER_H
