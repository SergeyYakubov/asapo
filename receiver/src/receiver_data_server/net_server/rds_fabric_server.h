#ifndef ASAPO_RDS_FABRIC_SERVER_H
#define ASAPO_RDS_FABRIC_SERVER_H

#include "rds_net_server.h"
#include "asapo/asapo_fabric/asapo_fabric.h"

namespace asapo {

class RdsFabricServer : public RdsNetServer {
  public:
    explicit RdsFabricServer(std::string  listenAddress, const AbstractLogger* logger);
    ~RdsFabricServer() override;

    // modified in testings to mock system calls, otherwise do not touch
    std::unique_ptr<fabric::FabricFactory> factory__;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
    std::unique_ptr<fabric::FabricServer> server__;
  private:
    std::string listenAddress_;
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
