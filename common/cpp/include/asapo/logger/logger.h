#ifndef ASAPO_LOGGER_H
#define ASAPO_LOGGER_H

#include <memory>
#include <string>

#include "asapo/common/error.h"
#include <ostream>

namespace asapo {

enum class LogLevel {
    None,
    Error,
    Info,
    Debug,
    Warning
};

class LogMessageWithFields {
  public:
    LogMessageWithFields(std::string key, uint64_t val);
    LogMessageWithFields(std::string key, double val, int precision);
    LogMessageWithFields(std::string val);
    LogMessageWithFields(const Error& error);
    LogMessageWithFields(std::string key, std::string val);
    LogMessageWithFields& Append(std::string key, uint64_t val);
    LogMessageWithFields& Append(std::string key, double val, int precision);
    LogMessageWithFields& Append(const LogMessageWithFields& log_msg);
    LogMessageWithFields& Append(std::string key, const LogMessageWithFields& log_msg);
    LogMessageWithFields& Append(std::string key, std::string val);
    std::string LogString() const;
  private:
    inline std::string CommaIfNeeded();
    std::string log_string_;
};


class AbstractLogger {
  public:
    virtual void SetLogLevel(LogLevel level) = 0;
    virtual void Info(const std::string& text) const = 0;
    virtual void Error(const std::string& text) const = 0;
    virtual void Debug(const std::string& text) const = 0;
    virtual void Warning(const std::string& text) const = 0;
    virtual void Info(const asapo::Error& error) const = 0;
    virtual void Error(const asapo::Error& error) const = 0;
    virtual void Debug(const asapo::Error& error) const = 0;
    virtual void Warning(const asapo::Error& error) const = 0;
    virtual void Info(const LogMessageWithFields& msg) const = 0;
    virtual void Error(const LogMessageWithFields& msg) const = 0;
    virtual void Debug(const LogMessageWithFields& msg) const = 0;
    virtual void Warning(const LogMessageWithFields& msg) const = 0;
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
