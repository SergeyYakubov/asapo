#ifndef ASAPO_TCP_CONNECTION_POOL_H
#define ASAPO_TCP_CONNECTION_POOL_H

#include <mutex>

#include "asapo/io/io.h"
#include "asapo/preprocessor/definitions.h"

namespace asapo {

struct TcpConnectionInfo {
    std::string uri;
    SocketDescriptor sd;
    bool in_use;
};

class TcpConnectionPool {
  public:
    ASAPO_VIRTUAL SocketDescriptor GetFreeConnection(const std::string& source, bool* reused, Error* err);
    ASAPO_VIRTUAL SocketDescriptor Reconnect(SocketDescriptor sd, Error* err);
    ASAPO_VIRTUAL  void ReleaseConnection(SocketDescriptor sd);
    ASAPO_VIRTUAL ~TcpConnectionPool() = default;
    TcpConnectionPool();
    std::unique_ptr<IO> io__;
  private:
    SocketDescriptor Connect(const std::string& source, Error* err);
    std::vector<TcpConnectionInfo> connections_;
    std::mutex mutex_;
};

}

#endif //ASAPO_TCP_CONNECTION_POOL_H
