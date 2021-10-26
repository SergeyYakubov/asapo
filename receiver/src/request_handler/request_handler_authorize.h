#ifndef ASAPO_REQUEST_HANDLER_AUTHORIZE_H
#define ASAPO_REQUEST_HANDLER_AUTHORIZE_H

#include <chrono>

#include "request_handler.h"
#include "asapo/logger/logger.h"
#include "asapo/http_client/http_client.h"
#include "authorization_client.h"


#include "asapo/io/io.h"

namespace asapo {

class RequestHandlerAuthorize : public ReceiverRequestHandler {
  public:
    RequestHandlerAuthorize()=delete;
    RequestHandlerAuthorize(AuthorizationData* authorization_cache);
    StatisticEntity GetStatisticEntity() const override;
    virtual Error ProcessRequest(Request* request) const override = 0;
    virtual ~RequestHandlerAuthorize()=default;
    const AbstractLogger* log__;
    std::unique_ptr<AuthorizationClient> auth_client__;
  protected:
    AuthorizationData* authorization_cache_;
    Error CheckVersion(const Request* request) const;
  private:
    void SetRequestFields(Request* request) const;
};

}

#endif //ASAPO_REQUEST_HANDLER_AUTHORIZE_H
