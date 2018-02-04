#include "curl_http_client.h"

#include <cstring>

namespace hidra2 {

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

size_t curl_write( void* ptr, size_t size, size_t nmemb, void* buffer) {
    auto strbuf = (std::string*)buffer;
    strbuf->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

void SetCurlOptions(CURL* curl, const std::string& uri, char* errbuf, std::string* buffer) {
    errbuf[0] = 0;
    curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 0L);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

}

HttpCode GetResponceCode(CURL* curl) {
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

WorkerErrorCode ProcessCurlResponce(CURL* curl, CURLcode res, const char* errbuf,
                                    std::string* buffer, HttpCode* responce_code) {
    if(res == CURLE_OK) {
        *responce_code = GetResponceCode(curl);
        return WorkerErrorCode::kOK;
    } else {
        *buffer = GetCurlError(curl, res, errbuf);
        return WorkerErrorCode::kErrorReadingSource;
    }
}

std::string CurlHttpClient::Get(const std::string& uri, HttpCode* responce_code, WorkerErrorCode* err) const noexcept {
    std::lock_guard<std::mutex> lock{mutex_};

    std::string buffer;
    char errbuf[CURL_ERROR_SIZE];
    SetCurlOptions(curl_, uri, errbuf, &buffer);

    auto res = curl_easy_perform(curl_);

    *err = ProcessCurlResponce(curl_, res, errbuf, &buffer, responce_code);

    return buffer;
}

CurlHttpClient::CurlHttpClient() {
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


}