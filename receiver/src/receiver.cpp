#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <common/networking.h>
#include "receiver.h"

const int hidra2::Receiver::kMaxUnacceptedConnectionsBacklog = 5;

void hidra2::Receiver::start_listener(std::string listener_address, uint16_t port) {
    if(listener_running_) {
        return;//Already listening
    }
    listener_running_ = true;

    sockaddr_in socket_address {};
    socket_address.sin_addr.s_addr = inet_addr(listener_address.c_str());
    socket_address.sin_port = htons(port);
    socket_address.sin_family = AF_INET;

    listener_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    bind(listener_fd_, reinterpret_cast<const sockaddr*>(&socket_address), sizeof(socket_address));
    listen(listener_fd_, kMaxUnacceptedConnectionsBacklog);

    listener_thread_ = new std::thread([this] {
        socklen_t sockaddr_in_size  = sizeof(sockaddr_in);
        while(listener_running_) {
            sockaddr_in client_address;
            int peer_fd = accept(listener_fd_, reinterpret_cast<sockaddr*>(&client_address), &sockaddr_in_size);
            char* address = inet_ntoa(client_address.sin_addr);

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
    std::cout << "New connection from " << address << std::endl;
    auto* request = new hidra2::GenericNetworkRequest();

    while(true) {
        recv(peer_socket_fd, request, sizeof(GenericNetworkRequest), 0);
    }
}
