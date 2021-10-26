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
  ~RequestHandlerSecondaryAuthorization()=default;
/* private:
  mutable AuthorizationData cached_auth_;
  mutable std::string cached_source_credentials_;
  mutable std::chrono::system_clock::time_point last_updated_;
  Error ProcessAuthorizationRequest(Request* request) const;
  Error ProcessOtherRequest(Request* request) const;
  Error ProcessReAuthorization(Request* request) const;
  bool NeedReauthorize() const;
  void SetRequestFields(Request* request) const;
  Error CheckVersion(const std::string& version_from_client) const;*/
};

}

#endif //ASAPO_RECEIVER_SRC_REQUEST_HANDLER_REQUEST_HANDLER_SECONDARY_AUTHORIZATION_H_
