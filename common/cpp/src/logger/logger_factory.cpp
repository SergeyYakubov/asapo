#include "spd_logger.h"

namespace hidra2 {

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


};