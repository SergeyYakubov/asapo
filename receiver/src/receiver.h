#ifndef HIDRA2_RECEIVER_H
#define HIDRA2_RECEIVER_H

#include <string>
#include <netinet/in.h>
#include <thread>
#include <system_wrappers/has_io.h>
#include "network_producer_peer.h"
#include <list>

namespace hidra2 {

enum class ReceiverError {
    NO_ERROR,
    ALREADY_LISTEING,
    FAILED_CREATING_SOCKET,
};

class Receiver : public HasIO {
    friend NetworkProducerPeer;
  private:
    bool listener_running_ = false;
    FileDescriptor listener_fd_;
    std::thread* accept_thread_ = nullptr;

    void AcceptThreadLogic();
    std::list<std::unique_ptr<NetworkProducerPeer>> peer_list_;
    std::unique_ptr<NetworkProducerPeer> on_new_peer_(int peer_socket_fd, std::string address);

  public:
    static const int kMaxUnacceptedConnectionsBacklog;

    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver() = default;

    void StartListener(std::string listener_address, uint16_t port, ReceiverError* err);
    void StopListener(ReceiverError* err);
};

}

#endif //HIDRA2_RECEIVER_H
