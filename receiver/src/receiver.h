#ifndef HIDRA2_RECEIVER_H
#define HIDRA2_RECEIVER_H

#include <string>
#include <thread>
#include <system_wrappers/has_io.h>
#include "network_producer_peer.h"
#include <list>

namespace hidra2 {

class Receiver : public HasIO {
  private:
    FileDescriptor listener_fd_ = -1;
    Error PrepareListener(std::string listener_address);
  public:
    static const int kMaxUnacceptedConnectionsBacklog;//TODO: Read from config

    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver() = default;

    void StartListener(std::string listener_address, Error* err);
    void AcceptThreadLogicWork(Error* err);
};

}

#endif //HIDRA2_RECEIVER_H
