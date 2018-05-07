#ifndef ASAPO_MOCKLOGGER_H
#define ASAPO_MOCKLOGGER_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "logger/logger.h"

namespace asapo {

class MockLogger : public AbstractLogger {
  public:
    MOCK_CONST_METHOD1(Info, void(const std::string&));
    MOCK_CONST_METHOD1(Error, void(const std::string& ));
    MOCK_CONST_METHOD1(Debug, void(const std::string& ));
    MOCK_CONST_METHOD1(Warning, void(const std::string& ));
    MOCK_METHOD1(SetLogLevel, void(LogLevel));
    MOCK_METHOD1(EnableLocalLog, void(bool));
    MOCK_METHOD1(EnableRemoteLog, void(bool));
};

}

#endif //ASAPO_MOCKLOGGER_H
