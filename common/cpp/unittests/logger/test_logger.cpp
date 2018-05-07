#include <gmock/gmock.h>
#include "gtest/gtest.h"

#include "../../src/logger/spd_logger.h"

using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;
using ::testing::_;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::HasSubstr;
using ::testing::ElementsAre;

using hidra2::LogLevel;

namespace {

void CheckConvert(const std::string& str, LogLevel level) {
    hidra2::Error err;
    auto loglev = hidra2::StringToLogLevel(str, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(loglev, Eq(level));
}

TEST(StringToLogLevel, ConvertOK) {
    CheckConvert("debug", LogLevel::Debug);
    CheckConvert("info", LogLevel::Info);
    CheckConvert("warning", LogLevel::Warning);
    CheckConvert("error", LogLevel::Error);
    CheckConvert("none", LogLevel::None);
}


TEST(StringToLogLevel, ConvertError) {
    hidra2::Error err;
    auto loglev = hidra2::StringToLogLevel("wrong", &err);
    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(loglev, Eq(hidra2::LogLevel::None));
}


TEST(DefaultLogger, BinLogger) {
    auto logger = hidra2::CreateDefaultLoggerBin("test");
    ASSERT_THAT(dynamic_cast<hidra2::SpdLogger*>(logger.get()), Ne(nullptr));
}

TEST(DefaultLogger, ApiLogger) {
    auto logger = hidra2::CreateDefaultLoggerApi("test", "endpoint");
    ASSERT_THAT(dynamic_cast<hidra2::SpdLogger*>(logger.get()), Ne(nullptr));
}

class MockSink : public spdlog::sinks::base_sink<std::mutex> {
  public:
    MockSink(const std::string& endpoint_uri) {};
  public:
    MOCK_METHOD1(_sink_it, void(
                     const spdlog::details::log_msg& msg));
    MOCK_METHOD0(_flush, void());
};

class LoggerTests : public Test {
  public:
    std::shared_ptr<MockSink> mock_sink{new MockSink{"test_url"}};
    std::unique_ptr<spdlog::logger> log;
    hidra2::SpdLogger logger{"test", "test_uri"};
    spdlog::details::log_msg msg;
    std::string test_string{"Hello"};
    void SetUp() override {
        msg.raw << test_string;
        log.reset(new spdlog::logger("mylogger", mock_sink));
        logger.log__ = std::move(log);
    }
    void TearDown() override {
    }
};

MATCHER_P(CompareMsg, msg, "") {
    if (arg.level != (*msg).level) return false;
    if (arg.raw.str() != (*msg).raw.c_str()) return false;

    return true;
}

TEST_F(LoggerTests, Info) {
    msg.level = spdlog::level::info;
    logger.SetLogLevel(LogLevel::Info);

    EXPECT_CALL(*mock_sink, _sink_it(CompareMsg(&msg)));

    logger.Info(test_string);
}

TEST_F(LoggerTests, Debug) {
    msg.level = spdlog::level::debug;
    logger.SetLogLevel(LogLevel::Debug);

    EXPECT_CALL(*mock_sink, _sink_it(CompareMsg(&msg)));

    logger.Debug(test_string);
}

TEST_F(LoggerTests, Error) {
    msg.level = spdlog::level::err;

    EXPECT_CALL(*mock_sink, _sink_it(CompareMsg(&msg)));

    logger.Error(test_string);
}

TEST_F(LoggerTests, Warning) {
    msg.level = spdlog::level::warn;

    EXPECT_CALL(*mock_sink, _sink_it(CompareMsg(&msg)));

    logger.Warning(test_string);
}

TEST_F(LoggerTests, NoWarningOnErrorLevel) {
    msg.level = spdlog::level::warn;
    logger.SetLogLevel(LogLevel::Error);

    EXPECT_CALL(*mock_sink, _sink_it(CompareMsg(&msg))).Times(0);

    logger.Warning(test_string);
}

TEST_F(LoggerTests, NoInfoOnWarningLevel) {
    msg.level = spdlog::level::info;
    logger.SetLogLevel(LogLevel::Warning);

    EXPECT_CALL(*mock_sink, _sink_it(CompareMsg(&msg))).Times(0);

    logger.Info(test_string);
}

TEST_F(LoggerTests, NoDebugOnNoneLevel) {
    msg.level = spdlog::level::debug;
    logger.SetLogLevel(LogLevel::None);

    EXPECT_CALL(*mock_sink, _sink_it(CompareMsg(&msg))).Times(0);

    logger.Info(test_string);
}

}

