#include "fabric_client_impl.h"
#include <rdma/fi_domain.h>
#include <cstring>

using namespace asapo;
using namespace fabric;

std::string FabricClientImpl::GetAddress() const {
    if (!domain_) {
        return "";
    }
    return FabricContextImpl::GetAddress();
}

std::unique_ptr<FabricMemoryRegion> FabricClientImpl::ShareMemoryRegion(void* src, size_t size, Error* error) {
    if (!domain_) {
        *error = FabricErrorTemplates::kClientNotInitializedError.Generate();
        return nullptr;
    }
    return FabricContextImpl::ShareMemoryRegion(src, size, error);
}

void FabricClientImpl::Send(FabricAddress dstAddress, FabricMessageId messageId, const void* src, size_t size,
                            Error* error) {
    if (!domain_) {
        *error = FabricErrorTemplates::kClientNotInitializedError.Generate();
        return;
    }
    FabricContextImpl::Send(dstAddress, messageId, src, size, error);
}

void FabricClientImpl::Recv(FabricAddress srcAddress, FabricMessageId messageId, void* dst, size_t size, Error* error) {
    if (!domain_) {
        *error = FabricErrorTemplates::kClientNotInitializedError.Generate();
        return;
    }
    FabricContextImpl::Recv(srcAddress, messageId, dst, size, error);
}

void
FabricClientImpl::RdmaWrite(FabricAddress dstAddress, const MemoryRegionDetails* details, const void* buffer, size_t size,
                            Error* error) {
    if (!domain_) {
        *error = FabricErrorTemplates::kClientNotInitializedError.Generate();
        return;
    }
    FabricContextImpl::RdmaWrite(dstAddress, details, buffer, size, error);
}

FabricAddress FabricClientImpl::AddServerAddress(const std::string& serverAddress, Error* error) {
    std::string hostname;
    uint16_t port;
    std::tie(hostname, port) = *io__->SplitAddressToHostnameAndPort(serverAddress);
    std::string serverIp = io__->ResolveHostnameToIp(hostname, error);

    InitIfNeeded(serverIp, error);
    if (*error) {
        return FI_ADDR_NOTAVAIL;
    }

    int result;
    FabricAddress addrIdx;

    result = fi_av_insertsvc(address_vector_, serverIp.c_str(), std::to_string(port).c_str(),
                             &addrIdx, 0, nullptr);
    if (result != 1) {
        *error = ErrorFromFabricInternal("fi_av_insertsvc", result);
        return FI_ADDR_NOTAVAIL;
    }

    FabricHandshakePayload handshake {};
    strcpy(handshake.hostnameAndPort, GetAddress().c_str());
    RawSend(addrIdx, &handshake, sizeof(handshake), error);
    if (*error) {
        return 0;
    }

    // Zero sized payload
    RawRecv(addrIdx, nullptr, 0, error);

    return addrIdx;
}

void FabricClientImpl::InitIfNeeded(const std::string& targetIpHint, Error* error) {
    const std::lock_guard<std::mutex> lock(initMutex_); // Will be released when scope is cleared

    if (domain_) {
        return; // Was already initialized
    }

    InitCommon(targetIpHint, 0, error);
}
