
#include <asapo/logger/logger.h>

#include "spd_logger.h"
#include "asapo/common/error.h"

namespace asapo {

Logger CreateLogger(std::string name, bool console, bool centralized_log, const std::string& endpoint_uri) {
    auto logger = new SpdLogger{name, endpoint_uri};
    logger->SetLogLevel(LogLevel::Info);
    if (console) {
        logger->EnableLocalLog(true);
    }
    if (centralized_log) {
        logger->EnableRemoteLog(true);
    }

    return Logger{logger};
}


Logger CreateDefaultLoggerBin(const std::string& name) {
    return CreateLogger(name, true, false, "");
}

Logger CreateDefaultLoggerApi(const std::string& name, const std::string& endpoint_uri) {
    return CreateLogger(name, false, true, endpoint_uri);
}

LogLevel StringToLogLevel(const std::string& name, Error* err) {
    *err = nullptr;
    if (name == "debug") return LogLevel::Debug;
    if (name == "info") return LogLevel::Info;
    if (name == "warning") return LogLevel::Warning;
    if (name == "none") return LogLevel::None;
    if (name == "error") return LogLevel::Error;

    *err = GeneralErrorTemplates::kSimpleError.Generate("wrong log level: " + name);
    return LogLevel::None;
}

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args ) {
    size_t size = static_cast<size_t>(snprintf( nullptr, 0, format.c_str(), args ... ) + 1);
    std::unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 );
}


std::string EncloseQuotes(std::string str) {
    return "\"" + std::move(str) + "\"";
}

LogMessageWithFields::LogMessageWithFields(std::string key, uint64_t val) {
    log_string_ = EncloseQuotes(key) + ":" + std::to_string(val);
}

LogMessageWithFields::LogMessageWithFields(std::string key, double val, int precision) {
    log_string_ = EncloseQuotes(key) + ":" + string_format("%." + std::to_string(precision) + "f", val);
}

LogMessageWithFields::LogMessageWithFields(std::string val) {
    log_string_ = EncloseQuotes("message") + ":" + EncloseQuotes(escape_json(val));
}

LogMessageWithFields::LogMessageWithFields(std::string key, std::string val) {
    log_string_ = EncloseQuotes(key) + ":" + EncloseQuotes(escape_json(val));
}

LogMessageWithFields& LogMessageWithFields::Append(std::string key, uint64_t val) {
    log_string_ += "," + EncloseQuotes(key) + ":" + std::to_string(val);
    return *this;
}

LogMessageWithFields& LogMessageWithFields::Append(std::string key, double val, int precision) {
    log_string_ += "," + EncloseQuotes(key) + ":" + string_format("%." + std::to_string(precision) + "f", val);
    return *this;
}

LogMessageWithFields& LogMessageWithFields::Append(std::string key, std::string val) {
    log_string_ += "," + EncloseQuotes(key) + ":" + EncloseQuotes(escape_json(val));
    return *this;
}

std::string LogMessageWithFields::LogString() const {
    return log_string_;
}

}
