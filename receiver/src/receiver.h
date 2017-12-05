#ifndef HIDRA2_RECEIVER_H
#define HIDRA2_RECEIVER_H

#include <string>
#include <netinet/in.h>
#include <thread>

namespace hidra2 {

enum {
    RECEIVER__ERR_OK,
    RECEIVER__ERR_UNK,
    RECEIVER__ERR_ADDRESS_IS_PROTECTED,
    RECEIVER__ERR_BAD_FILE_DESCRIPTOR,
    RECEIVER__ERR_ADDRESS_ALREADY_IN_USE,

};

class Receiver {
  private:
    static const int kMaxUnacceptedConnectionsBacklog;

    bool listener_running_ = false;
    std::thread* listener_thread_ = nullptr;
    int listener_fd_;

    void on_new_peer(int peer_socket_fd, std::string address);

  public:
    Receiver(const Receiver &) = delete;
    Receiver &operator=(const Receiver &) = delete;
    Receiver() = default;
    void start_listener(std::string listener_address, uint16_t port);
    void stop_listener();
};

}

#endif //HIDRA2_RECEIVER_H
