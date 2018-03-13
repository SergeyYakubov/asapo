#include <cstring>
#include <iostream>
#include "receiver.h"
#include "receiver_error.h"

const int hidra2::Receiver::kMaxUnacceptedConnectionsBacklog = 5;

void hidra2::Receiver::StartListener(std::string listener_address, Error* err) {
    *err = nullptr;

    if(listener_running_) {
        *err = ReceiverErrorTemplates::kAlreadyListening.Generate();
        return;
    }
    listener_running_ = true;

    FileDescriptor listener_fd = io->CreateSocket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP,
                                                  err);
    if(*err) {
        listener_running_ = false;
        return;
    }

    io->InetBind(listener_fd, listener_address, err);
    if(*err) {
        io->CloseSocket(listener_fd, nullptr);
        listener_running_ = false;
        return;
    }

    io->Listen(listener_fd, kMaxUnacceptedConnectionsBacklog, err);
    if(*err) {
        io->CloseSocket(listener_fd, nullptr);
        listener_running_ = false;
        return;
    }

    listener_fd_ = listener_fd;

    accept_thread_ = io->NewThread([this] {
        AcceptThreadLogic();
    });

}

void hidra2::Receiver::AcceptThreadLogic() {
    while(listener_running_) {
        std::string address;
        FileDescriptor peer_fd;

        Error err;
        auto client_info_tuple = io->InetAccept(listener_fd_, &err);
        if(err) {
            std::cerr << "An error occurred while accepting an incoming connection: " << err << std::endl;
            return;
        }
        std::tie(address, peer_fd) = *client_info_tuple;

        peer_list_.push_back(on_new_peer_(peer_fd, address));//TODO remove client when disconnect
    }
}

void hidra2::Receiver::StopListener(Error* err) {
    listener_running_ = false;
    io->CloseSocket(listener_fd_, nullptr);
    if(accept_thread_)
        accept_thread_->join();
    accept_thread_ = nullptr;
}

std::unique_ptr<hidra2::NetworkProducerPeer> hidra2::Receiver::on_new_peer_(int peer_socket_fd, std::string address) {
    NetworkProducerPeer* peer = new NetworkProducerPeer(peer_socket_fd, address);

    std::cout << "[" << peer->GetConnectionId() << "] New connection from " << address << std::endl;

    peer->start_peer_listener();

    return std::unique_ptr<hidra2::NetworkProducerPeer>(peer);
}
