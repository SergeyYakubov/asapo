#ifndef HIDRA2_FLUENTD_SINK_H
#define HIDRA2_FLUENTD_SINK_H

#include "spdlog/spdlog.h"
#include "spdlog/sinks/sink.h"
#include "spdlog/sinks/base_sink.h"
#include "http_client/http_client.h"

namespace hidra2 {

class FluentdSink : public spdlog::sinks::base_sink<std::mutex>
{
 public:
    FluentdSink(const std::string& endpoint_uri);
    std::unique_ptr<HttpClient> httpclient__;
 protected:
    void _sink_it(const spdlog::details::log_msg& msg) override;
    void _flush() override{};
 private:
    std::string endpoint_uri_;
};

}


#endif //HIDRA2_FLUENTD_SINK_H
