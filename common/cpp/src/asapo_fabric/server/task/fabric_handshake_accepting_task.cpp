#include <rdma/fi_endpoint.h>
#include "fabric_handshake_accepting_task.h"
#include "../fabric_server_impl.h"
#include "fabric_self_deleting_task.h"

using namespace asapo;
using namespace fabric;

FabricHandshakeAcceptingTask::~FabricHandshakeAcceptingTask() {
    DeleteRequest();
}

FabricHandshakeAcceptingTask::FabricHandshakeAcceptingTask(FabricServerImpl* server) : server_{server} {
}

void FabricHandshakeAcceptingTask::HandleCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) {
    Error error;
    HandleAccept(&error);
    if (error) {
        OnError(&error);
        return;
    }
    StartRequest();
}

void FabricHandshakeAcceptingTask::HandleErrorCompletion(fi_cq_err_entry* errEntry) {
    Error error;
    error = ErrorFromFabricInternal("FabricWaitableTask", -errEntry->err);
    OnError(&error);

    StartRequest();
}

void FabricHandshakeAcceptingTask::StartRequest() {
    if (server_->accepting_task_running) {
        Error error;
        server_->HandleFiCommand(fi_recv, this, &error,
                                 server_->endpoint_, &handshake_payload_, sizeof(handshake_payload_),
                                 nullptr, FI_ADDR_UNSPEC);

        if (error) {
            OnError(&error);
        }
    }
}

void FabricHandshakeAcceptingTask::DeleteRequest() {
    if (server_->endpoint_) { // The endpoint could not have been initialized
        fi_cancel(&server_->endpoint_->fid, this);

        // TODO Temporary fix:
        // Since the fi_cancel is not invoked instantly we have to wait a bit until the task was completed.
        // Theoretically we need a barrier(Promise) that is invoked on OnError.
        // And THEN there is also the possibility that we dont even exists in the queue,
        // so we have to timeout the promise too

        // Use verbs when testing this behavior!
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void FabricHandshakeAcceptingTask::HandleAccept(Error* error) {
    std::string hostname;
    uint16_t port;
    std::tie(hostname, port) = *server_->io__->SplitAddressToHostnameAndPort(handshake_payload_.hostnameAndPort);
    FabricAddress tmpAddr;
    int ret = fi_av_insertsvc(server_->address_vector_, hostname.c_str(), std::to_string(port).c_str(), &tmpAddr, 0,
                              nullptr);
    if (ret != 1) {
        *error = ErrorFromFabricInternal("fi_av_insertsvc", ret);
        return;
    }
    server_->log__->Debug("Got handshake from " + hostname + ":" + std::to_string(port));

    // TODO: This could slow down the whole complete queue process, maybe use another thread? :/
    // Send and forget
    server_->HandleFiCommand(fi_send, new FabricSelfDeletingTask(), error,
                             server_->endpoint_, nullptr, 0, nullptr, tmpAddr);
    if (*error) {
        return;
    }
}

void FabricHandshakeAcceptingTask::OnError(Error* error) {
    if (*error == FabricErrorTemplates::kInternalOperationCanceledError && !server_->accepting_task_running) {
        return; // The task was successfully canceled
    }
    server_->log__->Warning("AsapoFabric FabricHandshakeAcceptingTask: " + (*error)->Explain());
}
