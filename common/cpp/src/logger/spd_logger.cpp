#include "spd_logger.h"

#include "fluentd_sink.h"

#include <sstream>
#include <iomanip>


namespace asapo {

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

std::string escape_json(const std::string& s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        switch (*c) {
        case '"':
            o << "\\\"";
            break;
        case '\\':
            o << "\\\\";
            break;
        case '\b':
            o << "\\b";
            break;
        case '\f':
            o << "\\f";
            break;
        case '\n':
            o << "\\n";
            break;
        case '\r':
            o << "\\r";
            break;
        case '\t':
            o << "\\t";
            break;
        default:
            if ('\x00' <= *c && *c <= '\x1f') {
                o << "\\u"
                  << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
            } else {
                o << *c;
            }
        }
    }
    return o.str();
}

std::string EncloseMsg(std::string msg) {
    if (msg.find("\"") != 0) {
        return std::string(R"("message":")") + escape_json(msg) + "\"";
    } else {
        return msg;
    }

}

void SpdLogger::Info(const std::string& text)const {
    if (log__) {
        log__->info(EncloseMsg(text));
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
    log__ = std::unique_ptr<spdlog::logger> {new spdlog::async_logger(name_, std::begin(sinks_), std::end(sinks_), 1024)};
    log__->set_pattern(R"({"time":"%Y-%m-%d %H:%M:%S.%F","source":"%n","level":"%l",%v})");
}

SpdLogger::SpdLogger(const std::string& name, const std::string& endpoint_uri): name_{name}, endpoint_uri_{endpoint_uri} {

}
void SpdLogger::Error(const std::string& text)const {
    if (log__) {
        log__->error(EncloseMsg(text));
    }

}
void SpdLogger::Debug(const std::string& text) const {
    if (log__) {
        log__->debug(EncloseMsg(text));
    }

}

void SpdLogger::Warning(const std::string& text)const {
    if (log__) {
        log__->warn(EncloseMsg(text));
    }
}

void SpdLogger::EnableRemoteLog(bool enable) {
    centralized_log_ = enable;
    UpdateLoggerSinks();
}
void SpdLogger::Info(const LogMessageWithFields& msg) const {
    Info(msg.LogString());
}
void SpdLogger::Error(const LogMessageWithFields& msg) const {
    Error(msg.LogString());

}
void SpdLogger::Debug(const LogMessageWithFields& msg) const {
    Debug(msg.LogString());

}
void SpdLogger::Warning(const LogMessageWithFields& msg) const {
    Warning(msg.LogString());
}

}
