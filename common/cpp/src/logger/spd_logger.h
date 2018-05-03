#ifndef HIDRA2_SPDLOGGER_H
#define HIDRA2_SPDLOGGER_H

#include "logger/logger.h"
#include "spdlog/spdlog.h"

namespace hidra2 {

class SpdLogger : public AbstractLogger {
  public:
    explicit SpdLogger(const std::string& name, const std::string& endpoint_uri);
    void SetLogLevel(LogLevel level) override;
    void Info(const std::string& text) override;
    void Error(const std::string& text) override;
    void Debug(const std::string& text) override;
    void Warning(const std::string& text) override;
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
}

#endif //HIDRA2_SPDLOGGER_H
