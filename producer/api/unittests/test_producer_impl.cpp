#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockLogger.h"
#include "common/error.h"
#include "producer/common.h"
#include "../src/producer_impl.h"
#include "producer/producer_error.h"

#include "../src/request_pool.h"
#include "../src/request_handler_tcp.h"

#include "mocking.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::InSequence;
using ::testing::HasSubstr;


using asapo::RequestPool;
using asapo::Request;


MATCHER_P5(M_CheckSendDataRequest, op_code, beamtime_id, file_id, file_size, message,
           "Checks if a valid GenericRequestHeader was Send") {
    return ((asapo::GenericRequestHeader)(arg->header)).op_code == op_code
           && ((asapo::GenericRequestHeader)(arg->header)).data_id == file_id
           && ((asapo::GenericRequestHeader)(arg->header)).data_size == file_size
           && arg->beamtime_id == beamtime_id
           && strcmp(((asapo::GenericRequestHeader)(arg->header)).message, message) == 0;
}


TEST(ProducerImpl, Constructor) {
    asapo::ProducerImpl producer{"", 4, asapo::RequestHandlerType::kTcp};
    ASSERT_THAT(dynamic_cast<asapo::AbstractLogger*>(producer.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::RequestPool*>(producer.request_pool__.get()), Ne(nullptr));
}

class ProducerImplTests : public testing::Test {
  public:
    testing::NiceMock<MockDiscoveryService> service;
    asapo::RequestHandlerFactory factory{&service};
    testing::NiceMock<asapo::MockLogger> mock_logger;
    testing::NiceMock<MockRequestPull> mock_pull{&factory};
    asapo::ProducerImpl producer{"", 1, asapo::RequestHandlerType::kTcp};
    void SetUp() override {
        producer.log__ = &mock_logger;
        producer.request_pool__ = std::unique_ptr<RequestPool> {&mock_pull};
    }
    void TearDown() override {
        producer.request_pool__.release();
    }
};

TEST_F(ProducerImplTests, SendReturnsError) {
    EXPECT_CALL(mock_pull, AddRequest_t(_)).WillOnce(Return(
            asapo::ProducerErrorTemplates::kRequestPoolIsFull.Generate().release()));
    auto err = producer.Send(1, nullptr, 1, "", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kRequestPoolIsFull));
}

TEST_F(ProducerImplTests, ErrorIfFileNameTooLong) {
    std::string long_string(asapo::kMaxMessageSize+100, 'a');
    auto err = producer.Send(1, nullptr, 1, long_string, nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kFileNameTooLong));
}


TEST_F(ProducerImplTests, ErrorIfSizeTooLarge) {
    EXPECT_CALL(mock_logger, Error(testing::HasSubstr("error checking")));

    auto err = producer.Send(1, nullptr, asapo::ProducerImpl::kMaxChunkSize + 1, "", nullptr);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kFileTooLarge));
}


TEST_F(ProducerImplTests, OKSendingRequest) {
    uint64_t expected_size = 100;
    uint64_t expected_id = 10;
    char expected_name[asapo::kMaxMessageSize] = "test_name";
    std::string expected_beamtimeid = "beamtime_id";

    producer.SetBeamtimeId(expected_beamtimeid);
    Request request{"", asapo::GenericRequestHeader{asapo::kOpcodeTransferData, expected_id, expected_size, expected_name}, nullptr, nullptr};

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferData,
                                        expected_beamtimeid, expected_id, expected_size, expected_name))).WillOnce(Return(
                                                    nullptr));

    auto err = producer.Send(expected_id, nullptr, expected_size, expected_name, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(ProducerImplTests, ErrorSettingBeamtime) {
    std::string expected_beamtimeid(asapo::kMaxMessageSize*10,'a');
    EXPECT_CALL(mock_logger, Error(testing::HasSubstr("too long")));

    auto err = producer.SetBeamtimeId(expected_beamtimeid);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kBeamtimeIdTooLong));
}

TEST_F(ProducerImplTests, ErrorSettingSecondTime) {
    EXPECT_CALL(mock_logger, Error(testing::HasSubstr("already")));

    producer.SetBeamtimeId("1");
    auto err = producer.SetBeamtimeId("2");

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kBeamtimeAlreadySet));
}



}
