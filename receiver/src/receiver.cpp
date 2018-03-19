#include <cstring>
#include <iostream>
#include "receiver.h"
#include "receiver_error.h"

const int hidra2::Receiver::kMaxUnacceptedConnectionsBacklog = 5;

hidra2::Error hidra2::Receiver::PrepareListener(std::string listener_address) {
    Error err = nullptr;
    listener_fd_  = io->CreateAndBindIPTCPSocketListener(listener_address, kMaxUnacceptedConnectionsBacklog,
                    &err);
    return err;
}


void hidra2::Receiver::StartListener(std::string listener_address, Error* err) {
    *err = PrepareListener(listener_address);
    if(*err) {
        return;
    }

    while(true) {
        Error err_work = nullptr;
        AcceptThreadLogicWork(&err_work);
    }
}

void hidra2::Receiver::AcceptThreadLogicWork(hidra2::Error* err) {
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
    StartNewConnection(peer_fd, address);
}

