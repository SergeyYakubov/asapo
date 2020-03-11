#ifndef ASAPO_CURL_HTTP_CLIENT_H
#define ASAPO_CURL_HTTP_CLIENT_H

#include <string>
#include <mutex>
#include <ostream>

#include "http_client/http_client.h"
#include "curl/curl.h"
#include "io/io.h"

namespace asapo {

enum class CurlDataMode {
    string,
    array,
    file
};

struct CurlDataContainer {
    std::string string_buffer;
    FileData* p_array;
    uint64_t array_size;
    uint64_t bytes_received = 0;
    CurlDataMode mode;
    FileDescriptor fd;
    const IO* io;
};


class CurlHttpClient final : public HttpClient {
  public:
    CurlHttpClient();
    std::string Get(const std::string& uri, HttpCode* response_code, Error* err) const noexcept override;
    std::string Post(const std::string& uri, const std::string& cookie, const std::string& data, HttpCode* response_code,
                     Error* err) const noexcept override;
    Error Post(const std::string& uri,  const std::string& cookie, const std::string& input_data, FileData* output_data,
               uint64_t output_data_size,
               HttpCode* response_code)  const noexcept override;
    Error Post(const std::string& uri, const std::string& cookie,
               const std::string& input_data, std::string output_file_name,
               HttpCode* response_code)  const noexcept override;


    virtual ~CurlHttpClient();
  private:
    std::unique_ptr<IO> io_;
    Error Command(bool post, CurlDataContainer* data_container, const std::string& uri, const std::string& cookie,
                  const std::string& data, HttpCode* response_code) const noexcept;
    std::string StringPostGet(bool post, const std::string& uri, const std::string& cookie,
                              const std::string& data, HttpCode* response_code, Error* err) const noexcept;
    mutable std::mutex mutex_;
    CURL* curl_ = 0;
};



}

#endif //ASAPO_CURL_HTTP_CLIENT_H
