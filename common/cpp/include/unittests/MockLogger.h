#ifndef HIDRA2_MOCKLOGGER_H
#define HIDRA2_MOCKLOGGER_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "logger/logger.h"

namespace hidra2 {

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

#endif //HIDRA2_MOCKLOGGER_H
