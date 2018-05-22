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


MATCHER_P3(M_CheckSendDataRequest, file_id, file_size, file_name,
           "Checks if a valid GenericRequestHeader was Send") {
    return ((asapo::GenericRequestHeader*)arg)->op_code == asapo::kOpcodeTransferData
           && ((asapo::GenericRequestHeader*)arg)->data_id == file_id
           && std::string(((asapo::GenericRequestHeader*)arg)->file_name) == file_name
           && ((asapo::GenericRequestHeader*)arg)->data_size == file_size;
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

TEST_F(ProducerImplTests, ErrorIfSizeTooLarge) {
    auto err = producer.Send(1, nullptr, asapo::ProducerImpl::kMaxChunkSize + 1, "", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kFileTooLarge));
}


TEST_F(ProducerImplTests, OKSendingRequest) {
    uint64_t expected_size = 100;
    uint64_t expected_id = 10;
    std::string expected_name = "test_name";


    Request request{asapo::GenericRequestHeader{asapo::kOpcodeTransferData, expected_id, expected_size, expected_name}, nullptr, nullptr};

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(expected_id, expected_size, expected_name))).WillOnce(Return(
                nullptr));

    auto err = producer.Send(expected_id, nullptr, expected_size, expected_name, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}


}
