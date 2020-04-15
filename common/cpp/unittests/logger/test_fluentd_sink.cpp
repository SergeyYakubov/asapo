#include <gmock/gmock.h>
#include "gtest/gtest.h"

#include "../../src/logger/spd_logger.h"
#include "../../src/logger/fluentd_sink.h"

#include "unittests/MockHttpClient.h"
#include "http_client/http_error.h"

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
using ::testing::ElementsAre;
using asapo::MockHttpClient;
using asapo::FluentdSink;


namespace {

class FluentdSinkTests : public Test {
  public:
    std::shared_ptr<FluentdSink>sink{new FluentdSink{"test_url"}};
    NiceMock<MockHttpClient> mock_http_client;
    spdlog::details::log_msg msg;
    std::unique_ptr<spdlog::logger> logger;
    void SetUp() override {
        sink->httpclient__ = std::unique_ptr<asapo::HttpClient> {&mock_http_client};
        logger.reset(new spdlog::logger("mylogger", sink));
    }
    void TearDown() override {
        sink->httpclient__.release();
    }
};

TEST_F(FluentdSinkTests, SendPost) {
    EXPECT_CALL(mock_http_client, Post_t("test_url", HasSubstr("hello"), _, _));
    logger->info("hello");
}



}
