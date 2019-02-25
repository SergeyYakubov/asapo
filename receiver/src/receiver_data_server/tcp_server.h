#ifndef ASAPO_TCP_SERVER_H
#define ASAPO_TCP_SERVER_H

#include "net_server.h"
#include "io/io.h"
#include "logger/logger.h"
#include "receiver_data_server_request.h"
namespace asapo {

const int kMaxPendingConnections = 5;

class TcpServer : public NetServer {
  public:
    TcpServer(std::string address);
    ~TcpServer();
    GenericRequests GetNewRequests(Error* err) const noexcept override ;
    Error SendData(uint64_t source_id, void* buf, uint64_t size) const noexcept override;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
  private:
    void CloseSocket(SocketDescriptor socket) const noexcept;
    ListSocketDescriptors GetActiveSockets(Error* err) const noexcept;
    Error InitializeMasterSocketIfNeeded() const noexcept;
    ReceiverDataServerRequestPtr ReadRequest(SocketDescriptor socket, Error* err) const noexcept;
    GenericRequests ReadRequests(const ListSocketDescriptors& sockets) const noexcept;
    mutable SocketDescriptor master_socket_{kDisconnectedSocketDescriptor};
    mutable ListSocketDescriptors sockets_to_listen_;
    std::string address_;
};

}

#endif //ASAPO_TCP_SERVER_H
