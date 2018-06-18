#ifndef ASAPO_REQUEST_HANDLER_AUTHORIZE_H
#define ASAPO_REQUEST_HANDLER_AUTHORIZE_H

#include <chrono>

#include "request_handler.h"
#include "logger/logger.h"
#include "http_client/http_client.h"


#include "io/io.h"

namespace asapo {

class RequestHandlerAuthorize final: public RequestHandler {
  public:
  RequestHandlerAuthorize();
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    const AbstractLogger* log__;
    std::unique_ptr<HttpClient>http_client__;
 private:
    mutable std::string beamtime_id_;
    mutable std::chrono::high_resolution_clock::time_point last_updated_;
};

}

#endif //ASAPO_REQUEST_HANDLER_AUTHORIZE_H
