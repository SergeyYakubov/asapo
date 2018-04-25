#include "spd_logger.h"

#include "fluentd_sink.h"

namespace hidra2 {

void SpdLogger::SetLogLevel(LogLevel level) {
    if (log_) {
        switch (level){
            case LogLevel::None:
                log_->set_level(spdlog::level::off);
                break;
            case LogLevel::Debug:
                log_->set_level(spdlog::level::debug);
                break;
            case LogLevel::Error:
                log_->set_level(spdlog::level::err);
                break;
            case LogLevel::Warning:
                log_->set_level(spdlog::level::warn);
                break;
            case LogLevel::Info:
                log_->set_level(spdlog::level::info);
                break;
            default:
                log_->set_level(spdlog::level::err);
        }
    }
}

void SpdLogger::Info(const std::string &text) {
    if (log_) {
        log_->info(text);
    }
}

void SpdLogger::EnableConsoleLog(bool enable) {
    console_log_ = enable;
    UpdateLoggerSinks();
}

void SpdLogger::UpdateLoggerSinks() {
    sinks_.clear();
    if (console_log_) {
    sinks_.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
    }

    if (centralized_log_) {
        sinks_.push_back(std::make_shared<FluentdSink>(endpoint_uri_));
    }

    log_ = std::make_shared<spdlog::logger>(name_, std::begin(sinks_), std::end(sinks_));
}

SpdLogger::SpdLogger(const std::string& name,const std::string& endpoint_uri): name_{name},endpoint_uri_{endpoint_uri}
{

}
void SpdLogger::Error(const std::string &text) {
    if (log_) {
        log_->error(text);
    }

}
void SpdLogger::Debug(const std::string &text) {
    if (log_) {
        log_->debug(text);
    }

}

void SpdLogger::Warning(const std::string &text) {
    if (log_) {
        log_->warn(text);
    }
}

void SpdLogger::EnableCentralizedLog(bool enable) {
    centralized_log_ = enable;
    UpdateLoggerSinks();
}

}