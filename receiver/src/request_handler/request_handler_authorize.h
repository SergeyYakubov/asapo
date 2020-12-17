#ifndef ASAPO_REQUEST_HANDLER_AUTHORIZE_H
#define ASAPO_REQUEST_HANDLER_AUTHORIZE_H

#include <chrono>

#include "request_handler.h"
#include "asapo/logger/logger.h"
#include "asapo/http_client/http_client.h"


#include "asapo/io/io.h"

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
    mutable std::string stream_;
    mutable std::string beamline_;
    mutable std::string offline_path_;
    mutable std::string online_path_;
    mutable SourceType source_type_;
    mutable std::string cached_source_credentials_;
    mutable std::chrono::system_clock::time_point last_updated_;
    Error ProcessAuthorizationRequest(Request* request) const;
    Error ProcessOtherRequest(Request* request) const;
    Error Authorize(Request* request, const char* source_credentials) const;
    Error ErrorFromAuthorizationServerResponse(const Error& err, HttpCode code) const;
    Error ProcessReAuthorization(Request* request) const;
    bool NeedReauthorize() const;
    std::string GetRequestString(const Request* request, const char* source_credentials) const;
};

}

#endif //ASAPO_REQUEST_HANDLER_AUTHORIZE_H
