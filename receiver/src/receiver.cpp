#include <cstring>
#include <iostream>
#include "receiver.h"
#include "receiver_error.h"
#include "connection.h"

namespace hidra2 {


const int Receiver::kMaxUnacceptedConnectionsBacklog = 5;

Error Receiver::PrepareListener(std::string listener_address) {
    Error err = nullptr;
    listener_fd_  = io->CreateAndBindIPTCPSocketListener(listener_address, kMaxUnacceptedConnectionsBacklog,
                    &err);
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

//TODO: remove error since it is not used
void Receiver::ProcessConnections(Error* err) {
    std::string address;
    FileDescriptor peer_fd;

    //TODO: Use InetAcceptConnectionWithTimeout
    auto client_info_tuple = io->InetAcceptConnection(listener_fd_, err);
    if(*err) {
        //TODO: this can produce a lot of error messages
        std::cerr << "An error occurred while accepting an incoming connection: " << err << std::endl;
        return;
    }
    std::tie(address, peer_fd) = *client_info_tuple;
    StartNewConnectionInSeparateThread(peer_fd, address);
}

void Receiver::StartNewConnectionInSeparateThread(int connection_socket_fd, const std::string& address)  {
    auto thread = io->NewThread([connection_socket_fd, address] {
        auto connection = std::unique_ptr<Connection>(new Connection(connection_socket_fd, address));
        std::cout << "[" << connection->GetId() << "] New connection from " << address << std::endl;
        connection->Listen();
    });

    if (thread) {
        thread->detach();
    }
    return;

}

}