#ifndef ASAPO_MOCKLOGGER_H
#define ASAPO_MOCKLOGGER_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "asapo/logger/logger.h"

namespace asapo {

class MockLogger : public AbstractLogger {
  public:
    void Info(const asapo::Error& msg) const override {
        Info(msg->ExplainInJSON());
    };
    void Error(const asapo::Error& msg) const override {
        Error(msg->ExplainInJSON());
    };
    void Debug(const asapo::Error& msg) const override {
        Debug(msg->ExplainInJSON());
    };
    void Warning(const asapo::Error& msg) const override {
        Warning(msg->ExplainInJSON());
    };

    MOCK_METHOD(void, Info, (const std::string&), (const, override));
    MOCK_METHOD(void, Error, (const std::string&), (const, override));
    MOCK_METHOD(void, Debug, (const std::string&), (const, override));
    MOCK_METHOD(void, Warning, (const std::string&), (const, override));
    void Info(const LogMessageWithFields& msg) const override {
        Info(msg.LogString());
    };
    void Error(const LogMessageWithFields& msg) const override {
        Error(msg.LogString());
    };
    void Debug(const LogMessageWithFields& msg) const override {
        Debug(msg.LogString());
    };
    void Warning(const LogMessageWithFields& msg) const override {
        Warning(msg.LogString());
    };

    MOCK_METHOD(void, SetLogLevel, (LogLevel), (override));
    MOCK_METHOD(void, EnableLocalLog, (bool), (override));
    MOCK_METHOD(void, EnableRemoteLog, (bool), (override));
};

}

#endif //ASAPO_MOCKLOGGER_H
