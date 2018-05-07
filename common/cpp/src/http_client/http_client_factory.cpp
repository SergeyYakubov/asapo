#include "curl_http_client.h"

namespace asapo {

std::unique_ptr<HttpClient> DefaultHttpClient() {
    return std::unique_ptr<HttpClient> {new CurlHttpClient};
}


}
