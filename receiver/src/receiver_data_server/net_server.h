#ifndef ASAPO_NET_SERVER_H
#define ASAPO_NET_SERVER_H

#include "common/error.h"

#include "common.h"

namespace asapo {

class NetServer {
  public:
    virtual Requests GetNewRequests(Error* err) const noexcept = 0;
    virtual ~NetServer() = default;
};

}

#endif //ASAPO_NET_SERVER_H
