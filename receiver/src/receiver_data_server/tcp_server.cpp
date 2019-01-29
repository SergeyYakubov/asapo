#include "tcp_server.h"
#include "receiver_data_server_logger.h"

#include "io/io_factory.h"
#include "common/networking.h"

namespace asapo {

TcpServer::TcpServer(std::string address) : io__{GenerateDefaultIO()}, log__{GetDefaultReceiverDataServerLogger()},
    address_{std::move(address)} {}

Error TcpServer::InitializeMasterSocketIfNeeded() const noexcept {
    Error
    err;
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

void TcpServer::CloseSocket(SocketDescriptor socket) const noexcept {
    sockets_to_listen_.erase(std::remove(sockets_to_listen_.begin(), sockets_to_listen_.end(), socket),
                             sockets_to_listen_.end());
    io__->CloseSocket(socket, nullptr);
    log__->Debug("connection " + io__->AddressFromSocket(socket) + " closed");
}

Request TcpServer::ReadRequest(SocketDescriptor socket, Error* err) const noexcept {
    Request request{(uint64_t) socket, this};
    io__->Receive(socket, &request.header,
                  sizeof(GenericRequestHeader), err);
    if (*err == ErrorTemplates::kEndOfFile) {
        CloseSocket(socket);
    } else if (*err) {
        log__->Error("error getting next request from " + io__->AddressFromSocket(socket) + ": " + (*err)->
                     Explain()
                    );
    }
    return request;
}

Requests TcpServer::ReadRequests(const ListSocketDescriptors& sockets) const noexcept {
    Requests requests;
    for (auto client : sockets) {
        Error
        err;
        auto request = ReadRequest(client, &err);
        if (err) {
            continue;
        }
        requests.emplace_back(std::move(request));
    }
    return requests;
}

Requests TcpServer::GetNewRequests(Error* err) const noexcept {
    if (*err = InitializeMasterSocketIfNeeded()) {
        return {};
    }

    auto sockets = GetActiveSockets(err);
    if (*err) {
        return {};
    }

    return ReadRequests(sockets);
}

TcpServer::~TcpServer() {
    if (!io__) return; // need for test that override io__ to run
    for (auto client: sockets_to_listen_) {
        io__->CloseSocket(client, nullptr);
    }
    io__->CloseSocket(master_socket_, nullptr);
}


}