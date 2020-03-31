#ifndef ASAPO_RDS_FABRIC_SERVER_H
#define ASAPO_RDS_FABRIC_SERVER_H

#include "net_server.h"

namespace asapo {

class FabricServerRds : public NetServer {
public:
    explicit FabricServerRds(const std::string& address);
    ~FabricServerRds() override;
public: // NetServer implementation
    GenericRequests GetNewRequests(Error* err) const noexcept override;
    Error SendData(uint64_t source_id, void* buf, uint64_t size) const noexcept override;
    void HandleAfterError(uint64_t source_id) const noexcept override;
};

}

#endif //ASAPO_RDS_FABRIC_SERVER_H
