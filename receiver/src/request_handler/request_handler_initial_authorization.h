#ifndef ASAPO_RECEIVER_SRC_REQUEST_HANDLER_REQUEST_HANDLER_INITIAL_AUTHORIZATION_H_
#define ASAPO_RECEIVER_SRC_REQUEST_HANDLER_REQUEST_HANDLER_INITIAL_AUTHORIZATION_H_

#include "request_handler_authorize.h"

namespace asapo {

class RequestHandlerInitialAuthorization final: public RequestHandlerAuthorize {
 public:
  RequestHandlerInitialAuthorization(AuthorizationData* authorization_cache);
  Error ProcessRequest(Request* request) const override;
  ~RequestHandlerInitialAuthorization()=default;
};

}



#endif //ASAPO_RECEIVER_SRC_REQUEST_HANDLER_REQUEST_HANDLER_INITIAL_AUTHORIZATION_H_
