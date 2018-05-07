#ifndef ASAPO_REQUEST_HANDLER_H
#define ASAPO_REQUEST_HANDLER_H

#include "receiver_error.h"
#include "statistics.h"

namespace asapo {

class Request;

class RequestHandler {
  public:
    virtual Error ProcessRequest(const Request& request) const = 0;
    virtual StatisticEntity GetStatisticEntity() const  = 0;
    virtual ~RequestHandler() = default;
  private:
};

}

#endif //ASAPO_REQUEST_HANDLER_H
