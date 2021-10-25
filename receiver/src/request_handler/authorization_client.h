#ifndef ASAPO_RECEIVER_SRC_REQUEST_HANDLER_AUTHORIZATION_CLIENT_H_
#define ASAPO_RECEIVER_SRC_REQUEST_HANDLER_AUTHORIZATION_CLIENT_H_

#include "asapo/io/io.h"
#include "asapo/http_client/http_client.h"

namespace asapo {

struct AuthorizationData {
  std::string beamtime_id;
  std::string data_source;
  std::string beamline;
  std::string offline_path;
  std::string online_path;
  SourceType source_type;
};

class Request;
class AbstractLogger;

class AuthorizationClient {
 public:
  AuthorizationClient();
  Error Authorize(const Request* request, std::string source_credentials, AuthorizationData* data) const;
  const AbstractLogger* log__;
  std::unique_ptr<HttpClient>http_client__;
 private:
  Error ErrorFromAuthorizationServerResponse(const Error& err, const std::string response, HttpCode code) const;
  void SetRequestFields(Request* request) const;
  std::string GetRequestString(const Request* request, const std::string& source_credentials) const;
};

}

#endif //ASAPO_RECEIVER_SRC_REQUEST_HANDLER_AUTHORIZATION_CLIENT_H_
