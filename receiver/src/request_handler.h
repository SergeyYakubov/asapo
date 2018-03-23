#ifndef HIDRA2_REQUEST_HANDLER_H
#define HIDRA2_REQUEST_HANDLER_H

#include "request.h"
#include "receiver_error.h"

namespace hidra2 {

class Request;

class RequestHandler {
  public:
    virtual Error ProcessRequest(const Request& request) const = 0;
  private:
};

}

#endif //HIDRA2_REQUEST_HANDLER_H
