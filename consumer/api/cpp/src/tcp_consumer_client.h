#ifndef ASAPO_CONSUMER_TCP_CLIENT_H
#define ASAPO_CONSUMER_TCP_CLIENT_H

#include "net_client.h"
#include "asapo/io/io.h"
#include "tcp_connection_pool.h"

namespace asapo {

class TcpConsumerClient : public NetClient {
  public:
    explicit TcpConsumerClient();
    Error GetData(const MessageMeta* info, MessageData* data) override;
    std::unique_ptr<IO> io__;
    std::unique_ptr<TcpConnectionPool> connection_pool__;
  private:
    Error SendGetDataRequest(SocketDescriptor sd, const MessageMeta* info) const noexcept;
    Error ReconnectAndResendGetDataRequest(SocketDescriptor* sd, const MessageMeta* info) const noexcept;
    Error ReceiveResponce(SocketDescriptor sd) const noexcept;
    Error QueryCacheHasData(SocketDescriptor* sd, const MessageMeta* info, bool try_reconnect) const noexcept;
    Error ReceiveData(SocketDescriptor sd, const MessageMeta* info, MessageData* data) const noexcept;
};

}

#endif //ASAPO_CONSUMER_TCP_CLIENT_H
