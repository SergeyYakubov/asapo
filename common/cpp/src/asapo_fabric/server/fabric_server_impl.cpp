#include "fabric_server_impl.h"
#include "task/fabric_recv_any_task.h"
#include <rdma/fi_tagged.h>

using namespace asapo;
using namespace fabric;

FabricServerImpl::~FabricServerImpl() {
    accepting_task_.Stop();
}

FabricServerImpl::FabricServerImpl(const AbstractLogger* logger)
    : log__{logger}, accepting_task_(this) {
}

std::string FabricServerImpl::GetAddress() const {
    return FabricContextImpl::GetAddress();
}

std::unique_ptr<FabricMemoryRegion> FabricServerImpl::ShareMemoryRegion(void* src, size_t size, Error* error) {
    return FabricContextImpl::ShareMemoryRegion(src, size, error);
}

void FabricServerImpl::Send(FabricAddress dstAddress, FabricMessageId messageId, const void* src, size_t size,
                            Error* error) {
    FabricContextImpl::Send(dstAddress, messageId, src, size, error);
}

void FabricServerImpl::Recv(FabricAddress srcAddress, FabricMessageId messageId, void* dst, size_t size, Error* error) {
    FabricContextImpl::Recv(srcAddress, messageId, dst, size, error);
}

void
FabricServerImpl::RdmaWrite(FabricAddress dstAddress, const MemoryRegionDetails* details, const void* buffer,
                            size_t size,
                            Error* error) {
    FabricContextImpl::RdmaWrite(dstAddress, details, buffer, size, error);
}

void
FabricServerImpl::RecvAny(FabricAddress* srcAddress, FabricMessageId* messageId, void* dst, size_t size, Error* error) {
    FabricRecvAnyTask anyTask;
    HandleFiCommandAndWait(FI_ASAPO_ADDR_NO_ALIVE_CHECK, &anyTask, error,
                           fi_trecv, dst, size, nullptr, FI_ADDR_UNSPEC, static_cast<size_t>(0), kRecvTaggedAnyMatch);

    if (!(*error)) {
        if (anyTask.GetSource() == FI_ADDR_NOTAVAIL) {
            *error = FabricErrorTemplates::kInternalError.Generate("Source address is unavailable");
        }
        *messageId = anyTask.GetMessageId();
        *srcAddress = anyTask.GetSource();
    }
}

void FabricServerImpl::InitAndStartServer(const std::string& host, uint16_t port, Error* error) {
    InitCommon(host, port, error);

    if (!(*error)) {
        accepting_task_.Start();
    }
}
