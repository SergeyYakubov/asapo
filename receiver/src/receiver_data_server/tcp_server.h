#ifndef ASAPO_TCP_SERVER_H
#define ASAPO_TCP_SERVER_H

#include "net_server.h"
#include "io/io.h"
#include "logger/logger.h"

namespace asapo {

const int kMaxPendingConnections = 5;

class TcpServer : public NetServer {
  public:
    TcpServer(std::string address);
    virtual Requests GetNewRequests(Error* err) const noexcept override ;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
  private:
    ListSocketDescriptors GetActiveSockets(Error* err) const noexcept;
    Error InitializeMasterSocketIfNeeded() const noexcept;
    mutable SocketDescriptor master_socket_{kDisconnectedSocketDescriptor};
    mutable ListSocketDescriptors sockets_to_listen_;
    std::string address_;
};

}

#endif //ASAPO_TCP_SERVER_H
