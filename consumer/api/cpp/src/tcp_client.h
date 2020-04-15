#ifndef ASAPO_TCP_CLIENT_H
#define ASAPO_TCP_CLIENT_H

#include "net_client.h"
#include "io/io.h"
#include "tcp_connection_pool.h"

namespace asapo {

class TcpClient : public NetClient {
  public:
    explicit TcpClient();
    Error GetData(const FileInfo* info, FileData* data) const noexcept override;
    std::unique_ptr<IO> io__;
    std::unique_ptr<TcpConnectionPool> connection_pool__;
  private:
    Error SendGetDataRequest(SocketDescriptor sd, const FileInfo* info) const noexcept;
    Error ReconnectAndResendGetDataRequest(SocketDescriptor* sd, const FileInfo* info) const noexcept;
    Error ReceiveResponce(SocketDescriptor sd) const noexcept;
    Error QueryCacheHasData(SocketDescriptor* sd, const FileInfo* info, bool try_reconnect) const noexcept;
    Error ReceiveData(SocketDescriptor sd, const FileInfo* info, FileData* data) const noexcept;
};

}

#endif //ASAPO_TCP_CLIENT_H
