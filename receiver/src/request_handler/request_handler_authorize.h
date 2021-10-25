#ifndef ASAPO_REQUEST_HANDLER_AUTHORIZE_H
#define ASAPO_REQUEST_HANDLER_AUTHORIZE_H

#include <chrono>

#include "request_handler.h"
#include "asapo/logger/logger.h"
#include "asapo/http_client/http_client.h"
#include "authorization_client.h"


#include "asapo/io/io.h"

namespace asapo {

class RequestHandlerAuthorize final: public ReceiverRequestHandler {
  public:
    RequestHandlerAuthorize();
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    const AbstractLogger* log__;
    std::unique_ptr<AuthorizationClient> auth_client__;
  private:
    mutable AuthorizationData cached_auth_;
    mutable std::string cached_source_credentials_;
    mutable std::chrono::system_clock::time_point last_updated_;
    Error ProcessAuthorizationRequest(Request* request) const;
    Error ProcessOtherRequest(Request* request) const;
    Error ProcessReAuthorization(Request* request) const;
    bool NeedReauthorize() const;
    void SetRequestFields(Request* request) const;
    Error CheckVersion(const std::string& version_from_client) const;
};

}

#endif //ASAPO_REQUEST_HANDLER_AUTHORIZE_H
