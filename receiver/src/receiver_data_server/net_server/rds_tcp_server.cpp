#include "rds_tcp_server.h"
#include "../receiver_data_server_logger.h"

#include "io/io_factory.h"
#include "common/networking.h"

namespace asapo {

RdsTcpServer::RdsTcpServer(std::string address) : io__{GenerateDefaultIO()}, log__{GetDefaultReceiverDataServerLogger()},
    address_{std::move(address)} {}

Error RdsTcpServer::Initialize() {
    Error err;
    if (master_socket_ == kDisconnectedSocketDescriptor) {
        master_socket_ = io__->CreateAndBindIPTCPSocketListener(address_, kMaxPendingConnections, &err);
        if (!err) {
            log__->Info("Started TCP ReceiverDataServer at '" + address_ + "'");
        } else {
            log__->Error("TCP ReceiverDataServer cannot listen on " + address_ + ": " + err->Explain());
        }
    } else {
        err = TextError("Server was already initialized");
    }
    return err;
}

ListSocketDescriptors RdsTcpServer::GetActiveSockets(Error* err) {
    std::vector<std::string> new_connections;
    auto sockets = io__->WaitSocketsActivity(master_socket_, &sockets_to_listen_, &new_connections, err);
    for (auto& connection : new_connections) {
        log__->Debug("new connection from " + connection);
    }
    return sockets;
}

void RdsTcpServer::CloseSocket(SocketDescriptor socket) {
    sockets_to_listen_.erase(std::remove(sockets_to_listen_.begin(), sockets_to_listen_.end(), socket),
                             sockets_to_listen_.end());
    log__->Debug("connection " + io__->AddressFromSocket(socket) + " closed");
    io__->CloseSocket(socket, nullptr);
}

ReceiverDataServerRequestPtr RdsTcpServer::ReadRequest(SocketDescriptor socket, Error* err) {
    GenericRequestHeader header;
    io__->Receive(socket, &header,
                  sizeof(GenericRequestHeader), err);
    if (*err == ErrorTemplates::kEndOfFile) {
        CloseSocket(socket);
        return nullptr;
    } else if (*err) {
        log__->Error("error getting next request from " + io__->AddressFromSocket(socket) + ": " + (*err)->
                     Explain()
                    );
        return nullptr;
    }
    return ReceiverDataServerRequestPtr{new ReceiverDataServerRequest{header, (uint64_t) socket}};
}

GenericRequests RdsTcpServer::ReadRequests(const ListSocketDescriptors& sockets) {
    GenericRequests requests;
    for (auto client : sockets) {
        Error err;
        auto request = ReadRequest(client, &err);
        if (err) {
            continue;
        }
        log__->Debug("received request opcode: " + std::to_string(request->header.op_code) + " id: " + std::to_string(
                         request->header.data_id));
        requests.emplace_back(std::move(request));
    }
    return requests;
}

GenericRequests RdsTcpServer::GetNewRequests(Error* err) {
    auto sockets = GetActiveSockets(err);
    if (*err) {
        return {};
    }

    return ReadRequests(sockets);
}

RdsTcpServer::~RdsTcpServer() {
    if (!io__) return; // need for test that override io__ to run
    for (auto client : sockets_to_listen_) {
        io__->CloseSocket(client, nullptr);
    }
    io__->CloseSocket(master_socket_, nullptr);
}

void RdsTcpServer::HandleAfterError(uint64_t source_id) {
    CloseSocket(source_id);
}

Error RdsTcpServer::SendResponse(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response) {
    Error err;
    io__->Send(request->source_id, response, sizeof(*response), &err);
    if (err) {
        log__->Error("cannot send to consumer" + err->Explain());
    }
    return err;
}

Error
RdsTcpServer::SendResponseAndSlotData(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response,
                                      const CacheMeta* cache_slot) {
    Error err;

    err = SendResponse(request, response);
    if (err) {
        return err;
    }

    io__->Send(request->source_id, cache_slot->addr, cache_slot->size, &err);
    if (err) {
        log__->Error("cannot send slot to worker" + err->Explain());
    }
    return err;
}

}
