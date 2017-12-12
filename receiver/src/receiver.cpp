#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include "receiver.h"
#include "network_producer_peer.h"

const int hidra2::Receiver::kMaxUnacceptedConnectionsBacklog = 5;

void hidra2::Receiver::start_listener(std::string listener_address, uint16_t port, ReceiverError* err) {
    *err = ReceiverError::NO_ERROR;

    if(listener_running_) {
        *err = ReceiverError::ALREADY_LISTEING;
        return;
    }
    listener_running_ = true;

    IOErrors io_error;

    FileDescriptor listener_fd = io->create_socket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP, &io_error);
    if(io_error != IOErrors::NO_ERROR) {
        std::cerr << "Fail to create socket" << std::endl;
        return;
    }

    io->inet_bind(listener_fd, listener_address, port, &io_error);
    if(io_error != IOErrors::NO_ERROR) {
        io->deprecated_close(listener_fd);
        std::cerr << "Fail to bind socket" << std::endl;
        return;
    }

    io->listen(listener_fd, kMaxUnacceptedConnectionsBacklog, &io_error);
    if(io_error != IOErrors::NO_ERROR) {
        io->deprecated_close(listener_fd);
        std::cerr << "Fail to start listen" << std::endl;
        return;
    }

    listener_fd_ = listener_fd;

    accept_thread_ = new std::thread([this] {
        accept_thread_logic_();//TODO add peer to some list
    });

}

void hidra2::Receiver::accept_thread_logic_() {
    while(listener_running_) {
        std::string address;
        FileDescriptor peer_fd;

        IOErrors io_error;
        auto client_info_tuple = io->inet_accept(listener_fd_, &io_error);
        if(io_error != IOErrors::NO_ERROR) {
            std::cerr << "An error occurred while accepting an incoming connection" << std::endl;
            return;
        }
        std::tie(address, peer_fd) = *client_info_tuple;

        peer_list_.push_back(on_new_peer_(peer_fd, address));//TODO remove client when disconnect
    }
}

void hidra2::Receiver::stop_listener(ReceiverError* err) {
    close(listener_fd_);
    listener_running_ = false;
    if(accept_thread_)
        accept_thread_->join();
    accept_thread_ = nullptr;
}

std::unique_ptr<hidra2::NetworkProducerPeer> hidra2::Receiver::on_new_peer_(int peer_socket_fd, std::string address) {
    NetworkProducerPeer* peer = new NetworkProducerPeer(peer_socket_fd, address);

    std::cout << "[" << peer->connection_id() << "] New connection from " << address << std::endl;

    peer->start_peer_listener();

    return std::unique_ptr<hidra2::NetworkProducerPeer>(peer);
}
