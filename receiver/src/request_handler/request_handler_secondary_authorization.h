#ifndef ASAPO_RECEIVER_SRC_REQUEST_HANDLER_REQUEST_HANDLER_SECONDARY_AUTHORIZATION_H_
#define ASAPO_RECEIVER_SRC_REQUEST_HANDLER_REQUEST_HANDLER_SECONDARY_AUTHORIZATION_H_

#include <chrono>

#include "request_handler.h"
#include "asapo/logger/logger.h"
#include "asapo/http_client/http_client.h"
#include "authorization_client.h"
#include "request_handler_authorize.h"

#include "asapo/io/io.h"

namespace asapo {

class RequestHandlerSecondaryAuthorization final: public RequestHandlerAuthorize {
  public:
    RequestHandlerSecondaryAuthorization() = delete;
    RequestHandlerSecondaryAuthorization(AuthorizationData* authorization_cache);
    virtual Error ProcessRequest(Request* request) const override;
    ~RequestHandlerSecondaryAuthorization() = default;
  private:
    bool NeedReauthorize() const;
    void SetRequestFields(Request* request) const;
    Error ProcessReAuthorization(const Request* request) const;
    Error CheckRequest(const Request* request) const;
    Error ReauthorizeIfNeeded(const Request* request) const;
    void InvalidateAuthCache() const;

};

}

#endif //ASAPO_RECEIVER_SRC_REQUEST_HANDLER_REQUEST_HANDLER_SECONDARY_AUTHORIZATION_H_
