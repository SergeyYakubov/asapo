#ifndef HIDRA2_CURL_HTTP_CLIENT_H
#define HIDRA2_CURL_HTTP_CLIENT_H

#include <string>

#include "http_client.h"

namespace hidra2 {

class CurlHttpClient final : public HttpClient {
  public:
    CurlHttpClient();
    std::string Get(const std::string& uri, int* responce_code, WorkerErrorCode* err) const override;
};


}

#endif //HIDRA2_CURL_HTTP_CLIENT_H
