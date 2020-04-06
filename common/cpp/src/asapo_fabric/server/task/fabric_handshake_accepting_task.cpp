#include <rdma/fi_endpoint.h>
#include "fabric_handshake_accepting_task.h"
#include "../fabric_server_impl.h"
#include "../../common/task/fabric_self_deleting_task.h"

using namespace asapo;
using namespace fabric;

FabricHandshakeAcceptingTask::FabricHandshakeAcceptingTask(FabricServerImpl* parentServerContext)
: FabricSelfRequeuingTask(parentServerContext) {
}

FabricServerImpl* FabricHandshakeAcceptingTask::ServerContext() {
    return dynamic_cast<FabricServerImpl*>(ParentContext());
}

void FabricHandshakeAcceptingTask::RequeueSelf() {
    Error ignored;
    ServerContext()->HandleRawFiCommand(this, &ignored,
                                fi_recv, &handshake_payload_, sizeof(handshake_payload_), nullptr, FI_ADDR_UNSPEC);
}

void FabricHandshakeAcceptingTask::OnCompletion(const fi_cq_tagged_entry*, FabricAddress) {
    Error error;
    HandleAccept(&error);
    if (error) {
        OnError(&error);
        return;
    }
}

void FabricHandshakeAcceptingTask::OnErrorCompletion(const fi_cq_err_entry* errEntry) {
    Error error;
    error = ErrorFromFabricInternal("FabricWaitableTask", -errEntry->err);
    OnError(&error);
}

void FabricHandshakeAcceptingTask::HandleAccept(Error* error) {
    auto server = ServerContext();
    std::string hostname;
    uint16_t port;
    std::tie(hostname, port) =
            *(server->io__->SplitAddressToHostnameAndPort(handshake_payload_.hostnameAndPort));
    FabricAddress tmpAddr;
    int ret = fi_av_insertsvc(
            server->address_vector_,
            hostname.c_str(),
            std::to_string(port).c_str(),
            &tmpAddr,
            0,
            nullptr);
    if (ret != 1) {
        *error = ErrorFromFabricInternal("fi_av_insertsvc", ret);
        return;
    }
    server->log__->Debug("Got handshake from " + hostname + ":" + std::to_string(port));

    // TODO: This could slow down the whole complete queue process, maybe use another thread?
    // Send and forget
    server->HandleRawFiCommand(new FabricSelfDeletingTask(), error,
                                fi_send, nullptr, 0, nullptr, tmpAddr);
}

void FabricHandshakeAcceptingTask::OnError(const Error* error) {
    ServerContext()->log__->Warning("AsapoFabric FabricHandshakeAcceptingTask: " + (*error)->Explain());
}
