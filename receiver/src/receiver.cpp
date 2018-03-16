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
    //TODO maybe use atomic exchange...
    listener_running_ = true;

    FileDescriptor listener_fd = io->CreateAndBindIPTCPSocketListener(listener_address, kMaxUnacceptedConnectionsBacklog,
                                 err);

    if(*err) {
        listener_running_ = false;
        return;
    }

    listener_fd_ = listener_fd;

    accept_thread_ = io->NewThread([this] {
        AcceptThreadLogic();
    });

}

void hidra2::Receiver::StopListener(Error* err) {
    listener_running_ = false;
    io->CloseSocket(listener_fd_, err);
    if(accept_thread_)
        accept_thread_->join();
    accept_thread_ = nullptr;
}

void hidra2::Receiver::AcceptThreadLogic() {
    Error err;
    while(listener_running_) {
        err = nullptr;
        AcceptThreadLogicWork(&err);
    }
}

void hidra2::Receiver::AcceptThreadLogicWork(hidra2::Error* err) {
    std::string address;
    FileDescriptor peer_fd;

    //TODO: Use InetAcceptConnectionWithTimeout
    auto client_info_tuple = io->InetAcceptConnection(listener_fd_, err);
    if(*err) {
        std::cerr << "An error occurred while accepting an incoming connection: " << err << std::endl;
        return;
    }
    std::tie(address, peer_fd) = *client_info_tuple;

    peer_list_.push_back(CreateNewPeer(peer_fd, address));//TODO remove client when disconnect
}

std::unique_ptr<hidra2::NetworkProducerPeer> hidra2::Receiver::CreateNewPeer(int peer_socket_fd,
        const std::string& address) const noexcept {
    auto peer = NetworkProducerPeer::CreateNetworkProducerPeer(peer_socket_fd, address);

    std::cout << "[" << peer->GetConnectionId() << "] New connection from " << address << std::endl;

    peer->StartPeerListener();

    return peer;
}

const std::list<std::unique_ptr<hidra2::NetworkProducerPeer>>& hidra2::Receiver::GetConnectedPeers() {
    return peer_list_;
}
