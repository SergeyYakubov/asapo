#include "fluentd_sink.h"

#include <iostream>

namespace hidra2 {

void FluentdSink::_sink_it(const spdlog::details::log_msg& msg) {
    std::string log_str = msg.formatted.str();
    HttpCode code;
    log_str.erase(log_str.find_last_not_of("\n\r\t") + 1);
    std::string string_to_send = "json={\"message\":\"" + log_str + "\"}";
    Error err;
    httpclient__->Post(endpoint_uri_, string_to_send, &code, &err);
    if (err) {
        std::cerr << "cannot send logs - " + err->Explain() << std::endl;
    }
}

FluentdSink::FluentdSink(const std::string& endpoint_uri): httpclient__{DefaultHttpClient()}, endpoint_uri_{endpoint_uri} {

};


}
