#ifndef ASAPO_REQUEST_HANDLER_AUTHORIZE_H
#define ASAPO_REQUEST_HANDLER_AUTHORIZE_H

#include "request_handler.h"
#include "logger/logger.h"

#include "io/io.h"

namespace asapo {

class RequestHandlerAuthorize final: public RequestHandler {
  public:
  RequestHandlerAuthorize();
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(const Request& request) const override;
    const AbstractLogger* log__;
 private:
};

}

#endif //ASAPO_REQUEST_HANDLER_AUTHORIZE_H
