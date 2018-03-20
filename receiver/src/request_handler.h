#ifndef HIDRA2_REQUEST_HANDLER_H
#define HIDRA2_REQUEST_HANDLER_H

#include "request.h"
#include "receiver_error.h"

namespace hidra2 {


class RequestHandler {
    virtual Error ProcessRequest(const Request& request) = 0;
};

}

#endif //HIDRA2_REQUEST_HANDLER_H
