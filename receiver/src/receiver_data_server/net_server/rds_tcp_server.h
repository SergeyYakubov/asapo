#ifndef ASAPO_RDS_TCP_SERVER_H
#define ASAPO_RDS_TCP_SERVER_H

#include "rds_net_server.h"
#include "io/io.h"
#include "logger/logger.h"
#include "../receiver_data_server_request.h"
namespace asapo {

const int kMaxPendingConnections = 5;

class RdsTcpServer : public RdsNetServer {
  public:
    explicit RdsTcpServer(std::string address);
    ~RdsTcpServer() override;

    Error Initialize() override;

    GenericRequests GetNewRequests(Error* err) override;
    Error SendResponse(const ReceiverDataServerRequest* request,
                       const GenericNetworkResponse* response) override;
    Error SendResponseAndSlotData(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response,
                                  const CacheMeta* cache_slot) override;
    void HandleAfterError(uint64_t source_id) override;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
  private:
    void CloseSocket(SocketDescriptor socket);
    ListSocketDescriptors GetActiveSockets(Error* err);
    ReceiverDataServerRequestPtr ReadRequest(SocketDescriptor socket, Error* err) ;
    GenericRequests ReadRequests(const ListSocketDescriptors& sockets);
    SocketDescriptor master_socket_{kDisconnectedSocketDescriptor};
    ListSocketDescriptors sockets_to_listen_;
    std::string address_;
};

}

#endif //ASAPO_RDS_TCP_SERVER_H
