#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockLogger.h"
#include "common/error.h"
#include "producer/producer.h"
#include "../src/producer_impl.h"
#include "../src/request_pool.h"
#include "../src/request.h"

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


TEST(ProducerImpl, Constructor) {
    asapo::ProducerImpl producer{"", 4};
    ASSERT_THAT(dynamic_cast<asapo::AbstractLogger*>(producer.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::RequestPool*>(producer.request_pool__.get()), Ne(nullptr));
}

asapo::ReceiverDiscoveryService discovery{"", 1};

class MockRequestPull : public RequestPool {
  public:
    MockRequestPull() : RequestPool{1, 1, &discovery} {};
    asapo::Error AddRequest(std::unique_ptr<asapo::Request> request) override {
        if (request == nullptr) {
            return asapo::Error{AddRequest_t(nullptr)};
        }
        return asapo::Error{AddRequest_t(request.get())};
    }
    MOCK_METHOD1(AddRequest_t, asapo::ErrorInterface * (Request*));
};

class ProducerImplTests : public testing::Test {
  public:
    testing::NiceMock<asapo::MockLogger> mock_logger;
    testing::NiceMock<MockRequestPull> mock_pull;
    asapo::ProducerImpl producer{"", 1};
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
    auto err = producer.Send(1, nullptr, 1, nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kRequestPoolIsFull));
}

TEST_F(ProducerImplTests, ErrorIfSizeTooLarge) {
    auto err = producer.Send(1, nullptr, asapo::ProducerImpl::kMaxChunkSize + 1, nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kFileTooLarge));
}


MATCHER_P(M_CheckSendDataRequest, request_size,
          "Checks if a valid request was send (check size only)") {
    return ((Request*)arg)->GetMemoryRequitements() == request_size;
}


TEST_F(ProducerImplTests, OKSendingRequest) {
    uint64_t expected_size = 100;
    Request request{asapo::GenericNetworkRequestHeader{}, nullptr, nullptr};
    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(expected_size + sizeof(Request)))).WillOnce(Return(nullptr));

    auto err = producer.Send(1, nullptr, expected_size, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}


}
