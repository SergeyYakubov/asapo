#include "tcp_server.h"
#include "receiver_data_server_logger.h"

#include "io/io_factory.h"

namespace asapo {

TcpServer::TcpServer(std::string address) : io__{GenerateDefaultIO()}, log__{GetDefaultReceiverDataServerLogger()},
    address_{std::move(address)} {}

Error TcpServer::InitializeMasterSocketIfNeeded() const noexcept {
    Error err;
    if (master_socket_ == kDisconnectedSocketDescriptor) {
        master_socket_ = io__->CreateAndBindIPTCPSocketListener(address_, kMaxPendingConnections, &err);
        if (!err) {
            log__->Info("data server listening on " + address_);
        } else {
            log__->Error("dataserver cannot listen on " + address_ + ": " + err->Explain());
        }
    }
    return err;
}

Requests TcpServer::GetNewRequests(Error* err) const noexcept {
    if (*err = InitializeMasterSocketIfNeeded()) {
        return {};
    }

    return {};
}

}