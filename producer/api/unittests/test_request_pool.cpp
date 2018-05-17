#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>

#include "unittests/MockLogger.h"
#include "common/error.h"

#include "../src/request_handler_tcp.h"
#include "../src/request_pool.h"
#include "../src/receiver_discovery_service.h"
#include "../src/request_handler_factory.h"
#include "mocking.h"

#include "io/io_factory.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::AllOf;
using testing::DoAll;
using testing::NiceMock;
using ::testing::InSequence;
using ::testing::HasSubstr;
using testing::AtLeast;
using testing::Ref;

using asapo::RequestHandler;
using asapo::RequestPool;
using asapo::Error;
using asapo::ErrorInterface;
using asapo::Request;
using asapo::GenericNetworkRequestHeader;



class MockRequestHandlerFactory : public asapo::RequestHandlerFactory {
 public:
  MockRequestHandlerFactory(RequestHandler* request_handler):
      RequestHandlerFactory(asapo::RequestHandlerType::kTcp, nullptr)
  {
      request_handler_ = request_handler;
  }
  std::unique_ptr<RequestHandler> NewRequestHandler(uint64_t thread_id) override {
      return std::unique_ptr<RequestHandler>{request_handler_};
  }
 private:
    RequestHandler* request_handler_;
};



class RequestPoolTests : public testing::Test {
  public:
    NiceMock<MockRequestHandler>* mock_request_handler = new testing::NiceMock<MockRequestHandler>;
    NiceMock<asapo::MockLogger> mock_logger;
    MockRequestHandlerFactory request_handler_factory{mock_request_handler};
    const uint8_t nthreads = 1;
    const uint64_t max_size = 1024 * 1024 * 1024;
    asapo::RequestPool pool {nthreads, max_size, &request_handler_factory};
    std::unique_ptr<Request> request{new Request{GenericNetworkRequestHeader{}, nullptr, nullptr}};
    void SetUp() override {
        pool.log__ = &mock_logger;
    }
    void TearDown() override {
    }
};


TEST(RequestPool, Constructor) {
    NiceMock<MockDiscoveryService> ds;
    NiceMock<asapo::RequestHandlerFactory> request_handler_factory{asapo::RequestHandlerType::kTcp,&ds};

    asapo::RequestPool pool{4, 4, &request_handler_factory};

    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(pool.log__), Ne(nullptr));
}



TEST(RequestPool, AddRequestFailsDueToSize) {
    asapo::ReceiverDiscoveryService discovery{asapo::expected_endpoint, 1000};
    NiceMock<asapo::RequestHandlerFactory> request_handler_factory{asapo::RequestHandlerType::kTcp,&discovery};

    RequestPool pool{4, 0, &request_handler_factory};
    std::unique_ptr<Request> request{new Request{GenericNetworkRequestHeader{}, nullptr, nullptr}};

    auto err = pool.AddRequest(std::move(request));

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kRequestPoolIsFull));

}

TEST_F(RequestPoolTests, AddRequestDoesGoFurtherWhenNotReady) {

    EXPECT_CALL(*mock_request_handler, ReadyProcessRequest()).Times(AtLeast(1)).WillRepeatedly(Return(false));
    EXPECT_CALL(*mock_request_handler, PrepareProcessingRequestLocked()).Times(0);

    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RequestPoolTests, NRequestsInQueue) {
    auto nreq = pool.NRequestsInQueue();
    ASSERT_THAT(nreq, Eq(0));
}

void ExpectSend(MockRequestHandler* mock_handler,int ntimes = 1) {
    EXPECT_CALL(*mock_handler, ReadyProcessRequest()).Times(ntimes).WillRepeatedly(Return(true));
    EXPECT_CALL(*mock_handler, PrepareProcessingRequestLocked()).Times(ntimes);
    EXPECT_CALL(*mock_handler, ProcessRequestUnlocked_t(_)).Times(ntimes).WillRepeatedly(Return(nullptr));
    EXPECT_CALL(*mock_handler, TearDownProcessingRequestLocked_t(nullptr)).Times(ntimes);
}



TEST_F(RequestPoolTests, AddRequestCallsSend) {

    ExpectSend(mock_request_handler);

    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(RequestPoolTests, AddRequestCallsSendTwoRequests) {

    Request* request2 = new Request{GenericNetworkRequestHeader{}, nullptr, nullptr};

    ExpectSend(mock_request_handler,2);



    auto err1 = pool.AddRequest(std::move(request));
    request.reset(request2);
    auto err2 = pool.AddRequest(std::move(request));

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(err2, Eq(nullptr));
}



TEST_F(RequestPoolTests, FinishProcessingThreads) {
    EXPECT_CALL(mock_logger, Debug(HasSubstr("finishing thread"))).Times(nthreads);
}


}
