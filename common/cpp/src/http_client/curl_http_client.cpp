#include "curl_http_client.h"

#include <cstring>
#include "asapo/http_client/http_error.h"
#include  "asapo/common/data_structs.h"
#include "asapo/io/io_factory.h"

namespace asapo {

// use CurlHttpClientInstance and static variable to init curl on program start end cleanup on exit
class CurlHttpClientInstance {
  public:
    CurlHttpClientInstance();
    ~CurlHttpClientInstance();
};
static CurlHttpClientInstance instance;
CurlHttpClientInstance::CurlHttpClientInstance() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

CurlHttpClientInstance::~CurlHttpClientInstance() {
    curl_global_cleanup();
}

size_t curl_write( void* ptr, size_t size, size_t nmemb, void* data_container) {
    auto container = (CurlDataContainer*)data_container;
    size_t nbytes = size * nmemb;
    switch (container->mode) {
    case CurlDataMode::string:
        container->string_buffer.append((char*)ptr, nbytes);
        break;
    case CurlDataMode::array:
        if (container->bytes_received + nbytes > container->array_size) {
            return -1;
        }
        memcpy(container->p_array->get() + container->bytes_received, ptr, nbytes);
        container->bytes_received += nbytes;
        break;
    case CurlDataMode::file:
        Error err;
        container->io->Write(container->fd, ptr, nbytes, &err);
        if (err) {
            return -1;
        }
        break;
    }
    return nbytes;
}

void SetCurlOptions(CURL* curl, bool post, const std::string& cookie, const std::string& data, const std::string& uri,
                    char* errbuf,
                    CurlDataContainer* data_container) {
    errbuf[0] = 0;
    curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_container);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 0L);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    if (!cookie.empty()) {
        curl_easy_setopt(curl, CURLOPT_COOKIE, cookie.c_str());
    }

    //todo use a config parameter for this
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000L);

    if (post) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    } else {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    }
}

HttpCode GetResponseCode(CURL* curl) {
    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    return static_cast<HttpCode>(http_code);
}

std::string GetCurlError(CURL* curl, CURLcode res, const char* errbuf) {
    if (strlen(errbuf) > 0) {
        return errbuf;
    } else {
        return curl_easy_strerror(res);
    }
}

Error ProcessCurlResponse(CURL* curl, CURLcode res, const char* errbuf, HttpCode* response_code) {
    if(res == CURLE_OK) {
        *response_code = GetResponseCode(curl);
        return nullptr;
    } else {
        auto err_string = GetCurlError(curl, res, errbuf);
        if (res == CURLE_COULDNT_CONNECT || res == CURLE_COULDNT_RESOLVE_HOST) {
            return HttpErrorTemplates::kConnectionError.Generate(err_string);
        } else {
            return HttpErrorTemplates::kTransferError.Generate(err_string);
        }
    }
}

Error CurlHttpClient::Command(bool post, CurlDataContainer* data_container, const std::string& uri,
                              const std::string& cookie,
                              const std::string& data, HttpCode* response_code) const noexcept {
    std::lock_guard<std::mutex> lock{mutex_};
    char errbuf[CURL_ERROR_SIZE];
    SetCurlOptions(curl_, post, cookie, data, uri, errbuf, data_container);
    auto res = curl_easy_perform(curl_);
    return ProcessCurlResponse(curl_, res, errbuf, response_code);
}

MessageData AllocateMemory(uint64_t size, Error* err) {
    MessageData data;
    try {
        data = MessageData{new uint8_t[(size_t)size + 1 ]};
    } catch (...) {
        *err = ErrorTemplates::kMemoryAllocationError.Generate();
        return nullptr;
    }
    *err = nullptr;
    return data;
}

Error CurlHttpClient::Post(const std::string& uri,
                           const std::string& cookie,
                           const std::string& input_data,
                           MessageData* output_data,
                           uint64_t output_data_size,
                           HttpCode* response_code) const noexcept {
    Error err;
    CurlDataContainer data_container;
    data_container.mode = CurlDataMode::array;
    uint64_t extended_size = output_data_size + 10000; // for error messages
    *output_data = AllocateMemory(extended_size, &err);
    if (err) {
        return err;
    }
    data_container.p_array = output_data;
    data_container.array_size = extended_size;
    err = Command(true, &data_container, uri, cookie, input_data, response_code);
    if (!err) {
        if (*response_code == HttpCode::OK) {
            if (output_data_size != data_container.bytes_received) {
                return HttpErrorTemplates::kTransferError.Generate("received " +
                        std::to_string(data_container.bytes_received) + ", expected " + std::to_string(output_data_size) + "bytes");
            }
            (*output_data)[output_data_size] = 0; // for reinterpret cast to string worked
        } else {
            (*output_data)[data_container.bytes_received] = 0; // for reinterpret cast to string worked
        }
        return nullptr;
    } else {
        return err;
    }
}

Error CurlHttpClient::Post(const std::string& uri,
                           const std::string& cookie,
                           const std::string& input_data,
                           std::string output_file_name,
                           HttpCode* response_code) const noexcept {
    Error err;
    CurlDataContainer data_container;
    data_container.mode = CurlDataMode::file;
    data_container.io = io_.get();
    data_container.fd = io_->Open(output_file_name, IO_OPEN_MODE_CREATE | IO_OPEN_MODE_RW | IO_OPEN_MODE_SET_LENGTH_0,
                                  &err);
    if (err) {
        return err;
    }

    err = Command(true, &data_container, uri, cookie, input_data, response_code);
    io_->Close(data_container.fd, nullptr);
    if (!err) {
        return nullptr;
    } else {
        return err;
    }
}

std::string CurlHttpClient::StringPostGet(bool post,
                                          const std::string& uri,
                                          const std::string& cookie,
                                          const std::string& data,
                                          HttpCode* response_code, Error* err) const noexcept {
    CurlDataContainer data_container;
    data_container.mode = CurlDataMode::string;
    *err = Command(post, &data_container, uri, cookie, data, response_code);
    if (!*err) {
        return data_container.string_buffer;
    } else {
        return "";
    }
}

std::string CurlHttpClient::Get(const std::string& uri, HttpCode* response_code, Error* err) const noexcept {
    return StringPostGet(false, uri, "", "", response_code, err);
}

std::string CurlHttpClient::Post(const std::string& uri,
                                 const std::string& cookie,
                                 const std::string& data,
                                 HttpCode* response_code,
                                 Error* err) const noexcept {
    return StringPostGet(true, uri, cookie, data, response_code, err);
}


CurlHttpClient::CurlHttpClient(): io_{GenerateDefaultIO()} {
    curl_ = curl_easy_init();
    if (!curl_) {
        throw "Cannot initialize curl";
    }

}

CurlHttpClient::~CurlHttpClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
}
std::string CurlHttpClient::UrlEscape(const std::string &uri) const noexcept {
    if (!curl_) {
        return "";
    }
    char *output = curl_easy_escape(curl_, uri.c_str(), uri.size());
    if (output) {
        auto res = std::string(output);
        curl_free(output);
        return res;
    } else {
        return "";
    }
}

}
