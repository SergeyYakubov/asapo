#include "spd_logger.h"

#include "fluentd_sink.h"

namespace hidra2 {

void SpdLogger::SetLogLevel(LogLevel level) {
    if (log__) {
        switch (level) {
        case LogLevel::None:
            log__->set_level(spdlog::level::off);
            break;
        case LogLevel::Debug:
            log__->set_level(spdlog::level::debug);
            break;
        case LogLevel::Error:
            log__->set_level(spdlog::level::err);
            break;
        case LogLevel::Warning:
            log__->set_level(spdlog::level::warn);
            break;
        case LogLevel::Info:
            log__->set_level(spdlog::level::info);
            break;
        }
    }
}

void SpdLogger::Info(const std::string& text)const {
    if (log__) {
        log__->info(text);
    }
}

void SpdLogger::EnableLocalLog(bool enable) {
    console_log_ = enable;
    UpdateLoggerSinks();
}

void SpdLogger::UpdateLoggerSinks() {
    sinks_.clear();
    if (console_log_) {
        sinks_.push_back(std::shared_ptr<spdlog::sinks::stdout_sink_mt> {new spdlog::sinks::stdout_sink_mt()});
    }

    if (centralized_log_) {
        sinks_.push_back(std::shared_ptr<FluentdSink> {new FluentdSink(endpoint_uri_)});
    }

    log__ = std::unique_ptr<spdlog::logger> {new spdlog::logger(name_, std::begin(sinks_), std::end(sinks_))};
}

SpdLogger::SpdLogger(const std::string& name, const std::string& endpoint_uri): name_{name}, endpoint_uri_{endpoint_uri} {

}
void SpdLogger::Error(const std::string& text)const {
    if (log__) {
        log__->error(text);
    }

}
void SpdLogger::Debug(const std::string& text) const {
    if (log__) {
        log__->debug(text);
    }

}

void SpdLogger::Warning(const std::string& text)const {
    if (log__) {
        log__->warn(text);
    }
}

void SpdLogger::EnableRemoteLog(bool enable) {
    centralized_log_ = enable;
    UpdateLoggerSinks();
}

}