#ifndef ASAPO_TCP_SERVER_H
#define ASAPO_TCP_SERVER_H

#include "net_server.h"

namespace asapo {

class TcpServer : public NetServer {
  public:
    virtual Requests GetNewRequests(Error* err) const noexcept override ;
};

}

#endif //ASAPO_TCP_SERVER_H
