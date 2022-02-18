#include "rds_tcp_server.h"

#include <utility>
#include "../receiver_data_server_logger.h"
#include "../receiver_data_server_error.h"

#include "asapo/io/io_factory.h"
#include "asapo/common/networking.h"

namespace asapo {

    RdsTcpServer::RdsTcpServer(std::string address, const AbstractLogger* logger, asapo::SharedReceiverMonitoringClient  monitoring) : io__{GenerateDefaultIO()},
                                                                                log__{logger},
                                                                                address_{std::move(address)}, monitoring_{std::move(monitoring)} {}

Error RdsTcpServer::Initialize() {
    if (master_socket_ != kDisconnectedSocketDescriptor) {
        return GeneralErrorTemplates::kSimpleError.Generate("server was already initialized");
    }
    Error io_err;
    master_socket_ = io__->CreateAndBindIPTCPSocketListener(address_, kMaxPendingConnections, &io_err);
    if (!io_err) {
        log__->Info(LogMessageWithFields("started TCP data server").Append("address", address_));
    } else {
        auto err =
            ReceiverDataServerErrorTemplates::kServerError.Generate("cannot start TCP data server", std::move(io_err));
        err->AddDetails("address", address_);
        return err;
    }
    return nullptr;
}

ListSocketDescriptors RdsTcpServer::GetActiveSockets(Error* err) {
    std::vector<std::string> new_connections;
    auto sockets = io__->WaitSocketsActivity(master_socket_, &sockets_to_listen_, &new_connections, err);
    for (auto &connection: new_connections) {
        log__->Debug(LogMessageWithFields("new connection").Append("origin", connection));
    }
    return sockets;
}

void RdsTcpServer::CloseSocket(SocketDescriptor socket) {
    sockets_to_listen_.erase(std::remove(sockets_to_listen_.begin(), sockets_to_listen_.end(), socket),
                             sockets_to_listen_.end());
    log__->Debug(LogMessageWithFields("connection closed").Append("origin", io__->AddressFromSocket(socket)));
    io__->CloseSocket(socket, nullptr);
}

ReceiverDataServerRequestPtr RdsTcpServer::ReadRequest(SocketDescriptor socket, Error* err) {
    GenericRequestHeader header;

    Error io_err;
    *err = nullptr;

    RequestStatisticsPtr statistics{new RequestStatistics};
    statistics->StartTimer(kNetworkIncoming);
    uint64_t bytesReceived = io__->Receive(socket, &header, sizeof(GenericRequestHeader), &io_err);
    statistics->StopTimer();
    statistics->AddIncomingBytes(bytesReceived);

    if (io_err == GeneralErrorTemplates::kEndOfFile) {
        *err = std::move(io_err);
        CloseSocket(socket);
        return nullptr;
    } else if (io_err) {
        *err = ReceiverDataServerErrorTemplates::kServerError.Generate("error getting next request",std::move(io_err));
        (*err)->AddDetails("origin",io__->AddressFromSocket(socket));
        return nullptr;
    }
    return ReceiverDataServerRequestPtr{new ReceiverDataServerRequest{header, (uint64_t) socket, std::move(statistics)}};
}

GenericRequests RdsTcpServer::ReadRequests(const ListSocketDescriptors& sockets) {
    GenericRequests requests;
    for (auto client : sockets) {
        Error err;
        auto request = ReadRequest(client, &err);
        if (err) {
            continue;
        }
        log__->Debug(LogMessageWithFields("received request").
            Append("operation", OpcodeToString(request->header.op_code)).
            Append("id", request->header.data_id));
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
    CloseSocket(static_cast<int>(source_id));
}

Error RdsTcpServer::SendResponse(const ReceiverDataServerRequest *request, const GenericNetworkResponse *response) {
    Error io_err,err;
    auto socket= static_cast<int>(request->source_id);
    io__->Send(socket, response, sizeof(*response), &io_err);
    if (io_err) {
        err = ReceiverDataServerErrorTemplates::kServerError.Generate("error sending response",std::move(io_err));
        err->AddDetails("origin",io__->AddressFromSocket(socket));
    }
    return err;
}

Error
RdsTcpServer::SendResponseAndSlotData(const ReceiverDataServerRequest *request, const GenericNetworkResponse *response,
                                      const CacheMeta *cache_slot) {
    Error err;
    err = SendResponse(request, response);
    if (err) {
        return err;
    }
    Error io_err;
    auto socket= static_cast<int>(request->source_id);
    io__->Send(socket, cache_slot->addr, cache_slot->size, &io_err);
    if (io_err) {
        err = ReceiverDataServerErrorTemplates::kServerError.Generate("error sending slot data",std::move(io_err));
        err->AddDetails("origin",io__->AddressFromSocket(socket));
    }
    return err;
}

SharedReceiverMonitoringClient RdsTcpServer::Monitoring() {
    return monitoring_;
}

}
