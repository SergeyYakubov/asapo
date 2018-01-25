#include <cstring>
#include <iostream>
#include "receiver.h"
#include "network_producer_peer.h"

const int hidra2::Receiver::kMaxUnacceptedConnectionsBacklog = 5;

void hidra2::Receiver::StartListener(std::string listener_address, ReceiverError* err) {
    *err = ReceiverError::NO_ERROR;

    if(listener_running_) {
        *err = ReceiverError::ALREADY_LISTEING;
        return;
    }
    listener_running_ = true;

    IOErrors io_error;

    FileDescriptor listener_fd = io->CreateSocket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP,
                                                  &io_error);
    if(io_error != IOErrors::kNoError) {
        *err = ReceiverError::FAILED_CREATING_SOCKET;
        listener_running_ = false;
        std::cerr << "Fail to create socket" << std::endl;
        return;
    }

    io->InetBind(listener_fd, listener_address, &io_error);
    if(io_error != IOErrors::kNoError) {
        io->Close(listener_fd, nullptr);
        *err = ReceiverError::FAILED_CREATING_SOCKET;
        listener_running_ = false;
        std::cerr << "Fail to bind socket" << std::endl;
        return;
    }

    io->Listen(listener_fd, kMaxUnacceptedConnectionsBacklog, &io_error);
    if(io_error != IOErrors::kNoError) {
        io->Close(listener_fd, nullptr);
        *err = ReceiverError::FAILED_CREATING_SOCKET;
        listener_running_ = false;
        std::cerr << "Fail to start listen" << std::endl;
        return;
    }

    listener_fd_ = listener_fd;

    accept_thread_ = io->NewThread([this] {
        AcceptThreadLogic();//TODO add peer to some list
    });

}

void hidra2::Receiver::AcceptThreadLogic() {
    while(listener_running_) {
        std::string address;
        FileDescriptor peer_fd;

        IOErrors io_error;
        auto client_info_tuple = io->InetAccept(listener_fd_, &io_error);
        if(io_error != IOErrors::kNoError) {
            std::cerr << "An error occurred while accepting an incoming connection" << std::endl;
            return;
        }
        std::tie(address, peer_fd) = *client_info_tuple;

        peer_list_.push_back(on_new_peer_(peer_fd, address));//TODO remove client when disconnect
    }
}

void hidra2::Receiver::StopListener(ReceiverError* err) {
    listener_running_ = false;
    io->Close(listener_fd_, nullptr);
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
