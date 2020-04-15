#ifndef ASAPO_TCP_CONNECTION_POOL_H
#define ASAPO_TCP_CONNECTION_POOL_H

#include <mutex>

#include "io/io.h"
#include "preprocessor/definitions.h"

namespace asapo {

struct TcpConnectionInfo {
    std::string uri;
    SocketDescriptor sd;
    bool in_use;
};

class TcpConnectionPool {
  public:
    VIRTUAL SocketDescriptor GetFreeConnection(const std::string& source, bool* reused, Error* err);
    VIRTUAL SocketDescriptor Reconnect(SocketDescriptor sd, Error* err);
    VIRTUAL  void ReleaseConnection(SocketDescriptor sd);
    TcpConnectionPool();
    std::unique_ptr<IO> io__;
  private:
    SocketDescriptor Connect(const std::string& source, Error* err);
    std::vector<TcpConnectionInfo> connections_;
    std::mutex mutex_;
};

}

#endif //ASAPO_TCP_CONNECTION_POOL_H
