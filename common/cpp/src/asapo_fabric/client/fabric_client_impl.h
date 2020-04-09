#ifndef ASAPO_FABRIC_CLIENT_IMPL_H
#define ASAPO_FABRIC_CLIENT_IMPL_H

#include <asapo_fabric/asapo_fabric.h>
#include "../common/fabric_context_impl.h"

namespace asapo {
namespace fabric {

class FabricClientImpl : public FabricClient, public FabricContextImpl {
  private:
    std::mutex initMutex_;
  public: // Link to FabricContext
    std::string GetAddress() const override;

    std::unique_ptr<FabricMemoryRegion> ShareMemoryRegion(void* src, size_t size, Error* error) override;

    void Send(FabricAddress dstAddress, FabricMessageId messageId,
              const void* src, size_t size, Error* error) override;

    void Recv(FabricAddress srcAddress, FabricMessageId messageId,
              void* dst, size_t size, Error* error) override;

    void RdmaWrite(FabricAddress dstAddress,
                   const MemoryRegionDetails* details, const void* buffer, size_t size,
                   Error* error) override;
  public:
    FabricAddress AddServerAddress(const std::string& serverAddress, Error* error) override;

  private:
    void InitIfNeeded(const std::string& targetIpHint, Error* error);
};

}
}

#endif //ASAPO_FABRIC_CLIENT_IMPL_H
