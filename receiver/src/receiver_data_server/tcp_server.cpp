#include "tcp_server.h"
#include "receiver_data_server_logger.h"

#include "io/io_factory.h"
#include "common/networking.h"

namespace asapo {

TcpServer::TcpServer(std::string address) : io__{GenerateDefaultIO()}, log__{GetDefaultReceiverDataServerLogger()},
    address_{std::move(address)} {}

Error TcpServer::InitializeMasterSocketIfNeeded() const noexcept {
    Error err;
    if (master_socket_ == kDisconnectedSocketDescriptor) {
        master_socket_ = io__->CreateAndBindIPTCPSocketListener(address_, kMaxPendingConnections, &err);
        if (!err) {
            log__->Info("data server listening on " + address_);
            sockets_to_listen_.push_back(master_socket_);
        } else {
            log__->Error("dataserver cannot listen on " + address_ + ": " + err->Explain());
        }
    }
    return err;
}


ListSocketDescriptors TcpServer::GetActiveSockets(Error* err) const noexcept {
    std::vector<std::string> new_connections;
    auto sockets = io__->WaitSocketsActivity(master_socket_, &sockets_to_listen_, &new_connections, err);
    if (*err) {
        return {};
    }

    for (auto& connection : new_connections) {
        log__->Debug("new connection from " + connection);
    }
    return sockets;
}

Requests TcpServer::GetNewRequests(Error* err) const noexcept {
    if (*err = InitializeMasterSocketIfNeeded()) {
        return {};
    }

    auto sockets = GetActiveSockets(err);
    if (*err) {
        return {};
    }

    for (auto client: sockets) {
        GenericRequestHeader generic_request_header;
        io__-> Receive(client, &generic_request_header,
                       sizeof(GenericRequestHeader), err);
        if(*err) {
            log__->Error("error getting next request from " + io__->AddressFromSocket(client) + ": " + (*err)->
                Explain()
            );
            continue;
        }
    }


    return {Requests{Request{}}};
}


}