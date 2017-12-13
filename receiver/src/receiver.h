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
    FAILD_CREATING_SOCKET,
};

class Receiver : HasIO {
  private:
    static const int kMaxUnacceptedConnectionsBacklog;

    bool listener_running_ = false;
    FileDescriptor listener_fd_;
    std::thread* accept_thread_ = nullptr;

    void accept_thread_logic_();
    std::list<std::unique_ptr<NetworkProducerPeer>> peer_list_;
    std::unique_ptr<NetworkProducerPeer> on_new_peer_(int peer_socket_fd, std::string address);
  public:
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver() = default;

    void start_listener(std::string listener_address, uint16_t port, ReceiverError* err);
    void stop_listener(ReceiverError* err);
};

}

#endif //HIDRA2_RECEIVER_H
