#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>

#include "unittests/MockLogger.h"
#include "common/error.h"

#include "../../include/request/request_pool.h"
#include "../../include/request/request_handler_factory.h"
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
using asapo::GenericRequest;
using asapo::GenericRequestHeader;



class MockRequestHandlerFactory : public asapo::RequestHandlerFactory {
  public:
    MockRequestHandlerFactory(RequestHandler* request_handler):
        RequestHandlerFactory() {
        request_handler_ = request_handler;
    }
    std::unique_ptr<RequestHandler> NewRequestHandler(uint64_t thread_id, uint64_t* shared_counter) override {
        return std::unique_ptr<RequestHandler> {request_handler_};
    }
  private:
    RequestHandler* request_handler_;
};

class TestRequest : public GenericRequest {
 public:
  TestRequest(GenericRequestHeader header):GenericRequest(header){};
};


class RequestPoolTests : public testing::Test {
  public:
    NiceMock<MockRequestHandler>* mock_request_handler = new testing::NiceMock<MockRequestHandler>;
    NiceMock<asapo::MockLogger> mock_logger;
    MockRequestHandlerFactory request_handler_factory{mock_request_handler};
    const uint8_t nthreads = 1;
    asapo::RequestPool pool {nthreads, &request_handler_factory,&mock_logger};
    std::unique_ptr<GenericRequest> request{new TestRequest{GenericRequestHeader{}}};
    void SetUp() override {
    }
    void TearDown() override {
    }
};


TEST(RequestPool, Constructor) {
    NiceMock<asapo::MockLogger> mock_logger;
    MockRequestHandlerFactory factory(nullptr);

    EXPECT_CALL(mock_logger, Debug(HasSubstr("starting"))).Times(4);
    EXPECT_CALL(mock_logger, Debug(HasSubstr("finishing thread"))).Times(4);

    asapo::RequestPool pool{4, &factory,&mock_logger};
}

TEST_F(RequestPoolTests, AddRequestDoesNotGoFurtherWhenNotReady) {

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

void ExpectSend(MockRequestHandler* mock_handler, int ntimes = 1) {
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

    TestRequest* request2 = new TestRequest{GenericRequestHeader{}};

    ExpectSend(mock_request_handler, 2);



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
