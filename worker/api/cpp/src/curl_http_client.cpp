#include "curl_http_client.h"

#include "curl/curl.h"

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

size_t curl_write( void *ptr, size_t size, size_t nmemb, void *buffer)
{
    auto strbuf=(std::string*)buffer;
    strbuf->append((char*)ptr, size*nmemb);
    return size*nmemb;
}

std::string CurlHttpClient::Get(const std::string& uri, int* responce_code, WorkerErrorCode* err) const {
    auto curl = curl_easy_init();
    std::string buffer;
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        auto res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (responce_code) {
            *responce_code = http_code;
        }

        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        curl_easy_cleanup(curl);
    }

    return buffer;
}


CurlHttpClient::CurlHttpClient() {
}


}
