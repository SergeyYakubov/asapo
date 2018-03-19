#include "network_producer_peer_impl.h"

namespace hidra2 {

std::unique_ptr<NetworkProducerPeer> NetworkProducerPeer::CreateNetworkProducerPeer(SocketDescriptor socket_fd,
        const std::string& address) {
    return std::unique_ptr<NetworkProducerPeer>(new NetworkProducerPeerImpl(socket_fd, address));
}

void StartNewConnection(int peer_socket_fd, const std::string& address)  {
    auto peer = NetworkProducerPeer::CreateNetworkProducerPeer(peer_socket_fd, address);

    std::cout << "[" << peer->GetConnectionId() << "] New connection from " << address << std::endl;
    peer->StartPeerListener();
}


}
