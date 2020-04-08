#ifndef ASAPO_RDS_TCP_SERVER_H
#define ASAPO_RDS_TCP_SERVER_H

#include "rds_net_server.h"
#include "io/io.h"
#include "logger/logger.h"
#include "receiver_data_server_request.h"
namespace asapo {

const int kMaxPendingConnections = 5;

class TcpServer : public RdsNetServer {
  public:
    explicit TcpServer(std::string address);
    ~TcpServer() override;

    Error Initialize() override;

    GenericRequests GetNewRequests(Error* err) const noexcept override;
    Error SendResponse(const ReceiverDataServerRequest* request,
                       const GenericNetworkResponse* response) const noexcept override;
    Error SendResponseAndSlotData(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response,
                                  const CacheMeta* cache_slot) const noexcept override;
    void HandleAfterError(uint64_t source_id) const noexcept override;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
  private:
    void CloseSocket(SocketDescriptor socket) const noexcept;
    ListSocketDescriptors GetActiveSockets(Error* err) const noexcept;
    ReceiverDataServerRequestPtr ReadRequest(SocketDescriptor socket, Error* err) const noexcept;
    GenericRequests ReadRequests(const ListSocketDescriptors& sockets) const noexcept;
    mutable SocketDescriptor master_socket_{kDisconnectedSocketDescriptor};
    mutable ListSocketDescriptors sockets_to_listen_;
    std::string address_;
};

}

#endif //ASAPO_RDS_TCP_SERVER_H
