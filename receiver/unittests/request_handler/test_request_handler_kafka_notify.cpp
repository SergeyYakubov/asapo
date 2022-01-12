#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockKafkaClient.h"
#include "../../src/request_handler/request_factory.h"
#include "../receiver_mocking.h"

using namespace testing;
using namespace asapo;

namespace {

class KafkaNotifyHandlerTests : public Test {
  public:
    NiceMock<MockKafkaClient> kafka_client;
    RequestHandlerKafkaNotify handler{&kafka_client};
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    std::string expected_filename = std::string("processed") + asapo::kPathSeparator + "filename";
    std::string expected_offline_path = std::string("offline") + asapo::kPathSeparator + "path";
    CustomRequestData expected_custom_data {kDefaultIngestMode, 0, 0};
    const std::string expected_topic = "asapo";

    void SetUp() override {
        GenericRequestHeader request_header;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", nullptr});
        EXPECT_CALL(*mock_request, GetFileName()).Times(2).WillRepeatedly(Return(expected_filename));
        EXPECT_CALL(*mock_request, GetSourceType()).Times(2).WillRepeatedly(Return(SourceType::kProcessed));
        EXPECT_CALL(*mock_request, GetCustomData_t()).WillOnce(Return(expected_custom_data));
        EXPECT_CALL(*mock_request, GetOfflinePath()).WillOnce(ReturnRef(expected_offline_path));
        EXPECT_CALL(kafka_client, Send_t(HasSubstr(expected_filename), expected_topic)).WillOnce(Return(nullptr));
    }

    void TearDown() override {
    }
};

TEST_F(KafkaNotifyHandlerTests, KafkaNotifyOK) {
    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));
    Mock::VerifyAndClearExpectations(mock_request.get());
    Mock::VerifyAndClearExpectations(mock_request.get());
}
}