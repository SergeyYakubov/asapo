#ifndef HIDRA2_CURL_HTTP_CLIENT_H
#define HIDRA2_CURL_HTTP_CLIENT_H

#include <string>

#include "http_client.h"

namespace hidra2 {

class CurlHttpClient final : public HttpClient {
  public:
    std::string Get(const std::string& uri, HttpCode* responce_code, WorkerErrorCode* err) const noexcept override;
};


}

#endif //HIDRA2_CURL_HTTP_CLIENT_H
