#ifndef ASAPO_REQUEST_HANDLER_AUTHORIZE_H
#define ASAPO_REQUEST_HANDLER_AUTHORIZE_H

#include <chrono>

#include "request_handler.h"
#include "logger/logger.h"
#include "http_client/http_client.h"


#include "io/io.h"

namespace asapo {

class RequestHandlerAuthorize final: public ReceiverRequestHandler {
  public:
    RequestHandlerAuthorize();
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    const AbstractLogger* log__;
    std::unique_ptr<HttpClient>http_client__;
  private:
    mutable std::string beamtime_id_;
    mutable std::string beamline_;
    mutable std::chrono::high_resolution_clock::time_point last_updated_;
    Error ProcessAuthorizationRequest(Request* request) const;
    Error ProcessOtherRequest(Request* request) const;
    Error Authorize(Request* request, const char* beamtime_id) const;
    Error ErrorFromServerResponse(const Error& err, HttpCode code) const;

    std::string GetRequestString(const Request* request, const char* beamtime_id) const;
};

}

#endif //ASAPO_REQUEST_HANDLER_AUTHORIZE_H
