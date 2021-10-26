#ifndef ASAPO_RECEIVER_SRC_REQUEST_HANDLER_AUTHORIZATION_CLIENT_H_
#define ASAPO_RECEIVER_SRC_REQUEST_HANDLER_AUTHORIZATION_CLIENT_H_

#include "asapo/io/io.h"
#include "asapo/http_client/http_client.h"
#include "structs.h"
#include "asapo/preprocessor/definitions.h"

namespace asapo {

class Request;
class AbstractLogger;

class AuthorizationClient {
 public:
  AuthorizationClient();
  VIRTUAL Error Authorize(const Request* request, AuthorizationData* data) const;
  const AbstractLogger* log__;
  std::unique_ptr<HttpClient>http_client__;
  VIRTUAL ~AuthorizationClient()=default;
 private:
  Error ErrorFromAuthorizationServerResponse(const Error& err, const std::string response, HttpCode code) const;
  void SetRequestFields(Request* request) const;
  std::string GetRequestString(const Request* request, const std::string& source_credentials) const;
};

}

#endif //ASAPO_RECEIVER_SRC_REQUEST_HANDLER_AUTHORIZATION_CLIENT_H_
