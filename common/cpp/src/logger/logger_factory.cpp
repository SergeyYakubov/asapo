#include "spd_logger.h"

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

    *err = TextError("wrong log level: " + name);
    return LogLevel::None;
}

};