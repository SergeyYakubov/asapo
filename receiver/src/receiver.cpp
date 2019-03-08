#include <cstring>
#include <iostream>
#include "receiver.h"
#include "receiver_error.h"
#include "connection.h"
#include <io/io_factory.h>

#include "receiver_config.h"

namespace asapo {


const int Receiver::kMaxUnacceptedConnectionsBacklog = 5;

Receiver::Receiver(SharedCache cache): cache_{cache}, io__{GenerateDefaultIO()}, log__{GetDefaultReceiverLogger()} {

}

Error Receiver::PrepareListener(std::string listener_address) {
    Error err = nullptr;
    listener_fd_  = io__->CreateAndBindIPTCPSocketListener(listener_address, kMaxUnacceptedConnectionsBacklog,
                    &err);
    if (err) {
        log__->Error("prepare listener: " + err->Explain());
    }
    return err;
}


void Receiver::Listen(std::string listener_address, Error* err, bool exit_after_first_connection) {
    *err = PrepareListener(listener_address);
    if(*err) {
        return;
    }

    while(true) {
        ProcessConnections(err);
        if (exit_after_first_connection) break;
    }
}

void Receiver::ProcessConnections(Error* err) {
    std::string address;
    FileDescriptor connection_socket_fd;
    //TODO: Use InetAcceptConnectionWithTimeout
    auto client_info_tuple = io__->InetAcceptConnection(listener_fd_, err);
    if(*err) {
        //TODO: this can produce a lot of error messages
        log__->Error("accepting an incoming connection: " + (*err)->Explain());
        return;
    }
    std::tie(address, connection_socket_fd) = *client_info_tuple;
    StartNewConnectionInSeparateThread(connection_socket_fd, address);
}

void Receiver::StartNewConnectionInSeparateThread(int connection_socket_fd, const std::string& address)  {
    log__->Info("new connection from " + address);
    auto thread = io__->NewThread([connection_socket_fd, address, this] {
        auto connection = std::unique_ptr<Connection>(new Connection(connection_socket_fd, address, cache_, GetReceiverConfig()->tag));
        connection->Listen();
    });

    if (thread) {
        threads_.emplace_back(std::move(thread));
    }
    return;
}

}