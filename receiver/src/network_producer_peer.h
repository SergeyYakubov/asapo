#ifndef HIDRA2_NETWORKPRODUCERPEER_H
#define HIDRA2_NETWORKPRODUCERPEER_H

#include <string>
#include <map>
#include <utility>
#include <thread>
#include <common/networking.h>
#include <system_wrappers/has_io.h>
#include <iostream>
#include <atomic>
#include <vector>

namespace hidra2 {

class NetworkProducerPeer {
  public:
    virtual ~NetworkProducerPeer() = default;

    virtual void StartPeerListener() = 0;
    virtual void StopPeerListener() = 0;

    virtual uint32_t GetConnectionId() const = 0;
    virtual std::string GetAddress() const = 0;

    virtual void ReceiveAndSaveFile(uint64_t file_id, size_t file_size, Error* err) const noexcept = 0;

    static std::unique_ptr<NetworkProducerPeer> CreateNetworkProducerPeer(SocketDescriptor socket_fd,
            const std::string& address);
};

void StartNewConnection(int peer_socket_fd, const std::string& address);


}

#endif //HIDRA2_NETWORKPRODUCERPEER_H
