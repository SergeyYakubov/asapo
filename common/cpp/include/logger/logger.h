#ifndef ASAPO_LOGGER_H
#define ASAPO_LOGGER_H

#include <memory>
#include <string>

#include "common/error.h"

namespace asapo {

enum class LogLevel {
    None,
    Error,
    Info,
    Debug,
    Warning
};

class AbstractLogger {
  public:
    virtual void SetLogLevel(LogLevel level) = 0;
    virtual void Info(const std::string& text) const = 0;
    virtual void Error(const std::string& text) const = 0;
    virtual void Debug(const std::string& text) const = 0;
    virtual void Warning(const std::string& text) const = 0;
    virtual void EnableLocalLog(bool enable) = 0;
    virtual void EnableRemoteLog(bool enable) = 0;
    virtual ~AbstractLogger() = default;
};

using Logger = std::unique_ptr<AbstractLogger>;

Logger CreateDefaultLoggerBin(const std::string& name);
Logger CreateDefaultLoggerApi(const std::string& name, const std::string& endpoint_uri);

LogLevel StringToLogLevel(const std::string& name, Error* err);


}

#endif //ASAPO_LOGGER_H
