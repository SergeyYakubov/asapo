#ifndef HIDRA2_RECEIVER_H
#define HIDRA2_RECEIVER_H

#include <string>
#include <thread>
#include <system_wrappers/has_io.h>
#include "network_producer_peer.h"
#include <list>

namespace hidra2 {

class Receiver : public HasIO {
    friend NetworkProducerPeer;
  private:
    bool listener_running_ = false;
    FileDescriptor listener_fd_ = -1;
    std::unique_ptr<std::thread> accept_thread_ = nullptr;

    std::list<std::unique_ptr<NetworkProducerPeer>> peer_list_;
  public:
    static const int kMaxUnacceptedConnectionsBacklog;//TODO: Read from config

    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;

    Receiver() = default;

    void StartListener(std::string listener_address, Error* err);
    void StopListener(Error* err);

    std::unique_ptr<NetworkProducerPeer> CreateNewPeer(int peer_socket_fd, const std::string& address);
    const std::list<std::unique_ptr<NetworkProducerPeer>>& GetConnectedPeers();

    //Preparation for up coming thread pool implementation
    void AcceptThreadLogic();
    void AcceptThreadLogicWork(Error* err);
};

}

#endif //HIDRA2_RECEIVER_H
