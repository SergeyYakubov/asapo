#ifndef ASAPO_SPDLOGGER_H
#define ASAPO_SPDLOGGER_H

#include "logger/logger.h"
#include "spdlog/spdlog.h"

namespace asapo {

class SpdLogger : public AbstractLogger {
  public:
    explicit SpdLogger(const std::string& name, const std::string& endpoint_uri);
    void SetLogLevel(LogLevel level) override;
    void Info(const std::string& text) const override;
    void Error(const std::string& text) const override;
    void Debug(const std::string& text) const override;
    void Warning(const std::string& text) const override;
    void Info(const LogMessageWithFields& msg) const override;
    void Error(const LogMessageWithFields& msg) const override;
    void Debug(const LogMessageWithFields& msg) const override;
    void Warning(const LogMessageWithFields& msg) const override;

    void EnableLocalLog(bool enable) override;
    void EnableRemoteLog(bool enable) override;
    ~SpdLogger() = default;
    std::unique_ptr<spdlog::logger> log__;
  private:
    std::string name_;
    std::string endpoint_uri_;
    std::vector<spdlog::sink_ptr> sinks_;
    bool console_log_ = false;
    bool centralized_log_ = false;
    void UpdateLoggerSinks();
};

std::string EncloseMsg(std::string msg);

}




#endif //ASAPO_SPDLOGGER_H
