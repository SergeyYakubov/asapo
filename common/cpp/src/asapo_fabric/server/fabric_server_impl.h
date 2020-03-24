#ifndef ASAPO_FABRIC_SERVER_IMPL_H
#define ASAPO_FABRIC_SERVER_IMPL_H

#include <asapo_fabric/asapo_fabric.h>
#include "../common/fabric_context_impl.h"
#include "../fabric_factory_impl.h"
#include "task/fabric_handshake_accepting_task.h"

namespace asapo { namespace fabric {

class FabricServerImpl : public FabricServer, public FabricContextImpl {
    friend FabricFactoryImpl;
    friend class FabricHandshakeAcceptingTask;

private:
    const AbstractLogger* log__;
    std::unique_ptr<FabricHandshakeAcceptingTask> accepting_task_;
    bool accepting_task_running = false;

    void InitAndStartServer(const std::string& host, uint16_t port, Error* error);
public:
    ~FabricServerImpl() override;
    explicit FabricServerImpl(const AbstractLogger* logger);
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
    void RecvAny(FabricAddress* srcAddress, FabricMessageId* messageId, void* dst, size_t size, Error* error) override;
};

}}

#endif //ASAPO_FABRIC_SERVER_IMPL_H
