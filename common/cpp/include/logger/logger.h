#ifndef HIDRA2_LOGGER_H
#define HIDRA2_LOGGER_H

#include <memory>
#include <string>

namespace hidra2 {

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
    virtual void Info(const std::string& text) = 0;
    virtual void Error(const std::string& text) = 0;
    virtual void Debug(const std::string& text) = 0;
    virtual void Warning(const std::string& text) = 0;

};

using Logger = std::unique_ptr<AbstractLogger>;

Logger CreateDefaultLoggerBin(const std::string& name);
Logger CreateDefaultLoggerApi(const std::string& name,const std::string& endpoint_uri);

}

#endif //HIDRA2_LOGGER_H
