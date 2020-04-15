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

using asapo::LogLevel;
using asapo::LogMessageWithFields;

namespace {

void CheckConvert(const std::string& str, LogLevel level) {
    asapo::Error err;
    auto loglev = asapo::StringToLogLevel(str, &err);
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
    asapo::Error err;
    auto loglev = asapo::StringToLogLevel("wrong", &err);
    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(loglev, Eq(asapo::LogLevel::None));
}


TEST(DefaultLogger, BinLogger) {
    auto logger = asapo::CreateDefaultLoggerBin("test");
    ASSERT_THAT(dynamic_cast<asapo::SpdLogger*>(logger.get()), Ne(nullptr));
}

TEST(DefaultLogger, ApiLogger) {
    auto logger = asapo::CreateDefaultLoggerApi("test", "endpoint");
    ASSERT_THAT(dynamic_cast<asapo::SpdLogger*>(logger.get()), Ne(nullptr));
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
    asapo::SpdLogger logger{"test", "test_uri"};
    spdlog::details::log_msg msg;
    spdlog::details::log_msg msg_json;

    std::string test_string{"Hello"};
    std::string test_string_json{R"("Hello":"test","int":1,"double":123.234)"};

    void SetUp() override {
        msg.raw << R"("message":"Hello")";
        msg_json.raw << R"("Hello":"test","int":1,"double":123.234)";
        log.reset(new spdlog::logger("mylogger", mock_sink));
        logger.log__ = std::move(log);
    }
    void TearDown() override {
    }
};

MATCHER_P(CompareMsg, msg, "") {
    *result_listener << "Comparing " << "|" << arg.raw.str() << "|" << " and " << "|" << (*msg).raw.str() << "|" <<
                     "Level:" << arg.level << " and " << (*msg).level;
    if (arg.level != (*msg).level) return false;

    if (arg.raw.str() != (*msg).raw.str()) return false;

    return true;
}

TEST_F(LoggerTests, Info) {
    msg.level = spdlog::level::info;

    logger.SetLogLevel(LogLevel::Info);

    EXPECT_CALL(*mock_sink, _sink_it(CompareMsg(&msg)));

    logger.Info(test_string);
}

TEST_F(LoggerTests, InfoJson) {
    msg_json.level = spdlog::level::info;

    logger.SetLogLevel(LogLevel::Info);

    EXPECT_CALL(*mock_sink, _sink_it(CompareMsg(&msg_json)));

    logger.Info(test_string_json);
}

TEST_F(LoggerTests, InfoMessage) {
    msg_json.level = spdlog::level::info;

    asapo::LogMessageWithFields msg{"Hello", "test"};
    logger.SetLogLevel(LogLevel::Info);

    EXPECT_CALL(*mock_sink, _sink_it(CompareMsg(&msg_json)));

    logger.Info(msg.Append("int", 1).Append("double", 123.234, 3));
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

TEST(Message, ConstructorString) {
    asapo::LogMessageWithFields msg{"Hello", "test"};

    auto message = msg.LogString();

    ASSERT_THAT(message, Eq(R"("Hello":"test")"));
}


TEST(Message, ConstructorInt) {
    asapo::LogMessageWithFields msg{"Hello", 123};

    auto message = msg.LogString();

    ASSERT_THAT(message, Eq(R"("Hello":123)"));
}


TEST(Message, ConstructorDouble) {
    asapo::LogMessageWithFields msg{"Hello", 123.0, 1};

    auto message = msg.LogString();

    ASSERT_THAT(message, Eq(R"("Hello":123.0)"));

}

TEST(Message, AddString) {
    auto message = asapo::LogMessageWithFields{"Hello", "test"} .Append("test", "test").LogString();

    ASSERT_THAT(message, Eq(R"("Hello":"test","test":"test")"));
}

TEST(Message, AddInt) {
    asapo::LogMessageWithFields msg{"Hello", "test"};
    msg = msg.Append("test", 123);

    auto message = msg.LogString();

    ASSERT_THAT(message, Eq(R"("Hello":"test","test":123)"));
}

TEST(Message, AddDouble) {
    asapo::LogMessageWithFields msg{"Hello", "test"};

    auto message = msg.Append("test", 123.2, 2).LogString();

    ASSERT_THAT(message, Eq(R"("Hello":"test","test":123.20)"));
}

TEST(Message, Multi) {
    asapo::LogMessageWithFields msg{"Hello", "test"};
    msg.Append("test", 123).Append("test", "test").Append("test", 123.2, 2);

    auto message = msg.LogString();

    ASSERT_THAT(message, Eq(R"("Hello":"test","test":123,"test":"test","test":123.20)"));
}


}

