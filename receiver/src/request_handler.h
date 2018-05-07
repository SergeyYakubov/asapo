#ifndef HIDRA2_REQUEST_HANDLER_H
#define HIDRA2_REQUEST_HANDLER_H

#include "receiver_error.h"
#include "statistics.h"

namespace hidra2 {

class Request;

class RequestHandler {
  public:
    virtual Error ProcessRequest(const Request& request) const = 0;
    virtual StatisticEntity GetStatisticEntity() const  = 0;
    virtual ~RequestHandler() = default;
  private:
};

}

#endif //HIDRA2_REQUEST_HANDLER_H
