#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <common/networking.h>
#include "receiver.h"
#include "network_producer_peer.h"

const int hidra2::Receiver::kMaxUnacceptedConnectionsBacklog = 5;

void hidra2::Receiver::start_listener(std::string listener_address, uint16_t port) {
    if(listener_running_) {
        return;//Already listening
    }
    listener_running_ = true;

    IOErrors err;

    FileDescriptor listener_fd = io->create_socket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP, &err);
    if(err != IOErrors::NO_ERROR) {
        std::cerr << "Fail to create socket" << std::endl;
    }

    io->inet_bind(listener_fd, listener_address, port, &err);
    if(err != IOErrors::NO_ERROR) {
        io->deprecated_close(listener_fd);
        std::cerr << "Fail to bind socket" << std::endl;
    }

    io->listen(listener_fd, kMaxUnacceptedConnectionsBacklog, &err);
    if(err != IOErrors::NO_ERROR) {
        io->deprecated_close(listener_fd);
        std::cerr << "Fail to start listen" << std::endl;
    }

    listener_fd_ = listener_fd;

    listener_thread_ = new std::thread([this] {
        socklen_t sockaddr_in_size  = sizeof(sockaddr_in);
        while(listener_running_) {
            std::string address;
            FileDescriptor peer_fd;

            IOErrors err;
            auto client_info_tuple = io->inet_accept(listener_fd_, &err);
            if(err != IOErrors::NO_ERROR) {
                std::cerr << "An error occurred while accepting an incoming connection" << std::endl;
                return;
            }
            std::tie(address, peer_fd) = *client_info_tuple;

            on_new_peer(peer_fd, address);
        }
    });

}

void hidra2::Receiver::stop_listener() {
    close(listener_fd_);
    listener_running_ = false;
    if(listener_thread_)
        listener_thread_->join();
    listener_thread_ = nullptr;
}

void hidra2::Receiver::on_new_peer(int peer_socket_fd, std::string address) {
    NetworkProducerPeer peer(peer_socket_fd, address);

    std::cout << "[" << peer.connection_id() << "] New connection from " << address << std::endl;

    peer.start_peer_receiver();
}
