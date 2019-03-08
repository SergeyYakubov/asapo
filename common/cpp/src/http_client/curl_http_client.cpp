#include "curl_http_client.h"

#include <cstring>

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

size_t curl_write( void* ptr, size_t size, size_t nmemb, void* buffer) {
    auto strbuf = (std::string*)buffer;
    strbuf->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

void SetCurlOptions(CURL* curl, bool post, const std::string& data, const std::string& uri, char* errbuf,
                    std::string* buffer) {
    errbuf[0] = 0;
    curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 0L);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    //todo use a config parameter for this
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000L);

    if (post) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
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

Error ProcessCurlResponse(CURL* curl, CURLcode res, const char* errbuf,
                          std::string* buffer, HttpCode* response_code) {
    if(res == CURLE_OK) {
        *response_code = GetResponseCode(curl);
        return nullptr;
    } else {
        *buffer = GetCurlError(curl, res, errbuf);
        return TextError("Curl client error: " + *buffer);
    }
}

std::string CurlHttpClient::Command(bool post, const std::string& uri, const std::string& data, HttpCode* response_code,
                                    Error* err) const noexcept {
    std::lock_guard<std::mutex> lock{mutex_};

    std::string buffer;
    char errbuf[CURL_ERROR_SIZE];

    SetCurlOptions(curl_, post, data, uri, errbuf, &buffer);

    auto res = curl_easy_perform(curl_);

    *err = ProcessCurlResponse(curl_, res, errbuf, &buffer, response_code);

    return buffer;

}


std::string CurlHttpClient::Get(const std::string& uri, HttpCode* response_code, Error* err) const noexcept {
    return Command(false, uri, "", response_code, err);
}

std::string CurlHttpClient::Post(const std::string& uri, const std::string& data, HttpCode* response_code,
                                 Error* err) const noexcept {
    return Command(true, uri, data, response_code, err);
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
