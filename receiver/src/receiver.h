#ifndef HIDRA2_RECEIVER_H
#define HIDRA2_RECEIVER_H

#include <string>
#include <thread>
#include <system_wrappers/has_io.h>
#include "connection.h"
#include <list>

namespace hidra2 {

class Receiver : public HasIO {
  private:
    FileDescriptor listener_fd_ = -1;
    Error PrepareListener(std::string listener_address);
    void StartNewConnectionInSeparateThread(int connection_socket_fd, const std::string& address);
    void ProcessConnections(Error* err);
  public:
    static const int kMaxUnacceptedConnectionsBacklog;//TODO: Read from config

    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver() = default;

    void Listen(std::string listener_address, Error* err, bool exit_after_first_connection = false);
};

}

#endif //HIDRA2_RECEIVER_H
