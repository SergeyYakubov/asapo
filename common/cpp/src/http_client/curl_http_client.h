#ifndef ASAPO_CURL_HTTP_CLIENT_H
#define ASAPO_CURL_HTTP_CLIENT_H

#include <string>
#include <mutex>

#include "http_client/http_client.h"
#include "curl/curl.h"

namespace asapo {

class CurlHttpClient final : public HttpClient {
  public:
    CurlHttpClient();
    std::string Get(const std::string& uri, HttpCode* response_code, Error* err) const noexcept override;
    std::string Post(const std::string& uri, const std::string& data, HttpCode* response_code,
                     Error* err) const noexcept override;
    virtual ~CurlHttpClient();
  private:
    std::string Command(bool post, const std::string& uri, const std::string& data, HttpCode* response_code,
                        Error* err) const noexcept;
    mutable std::mutex mutex_;
    CURL* curl_ = 0;
};


}

#endif //ASAPO_CURL_HTTP_CLIENT_H
