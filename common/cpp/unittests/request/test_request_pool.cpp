#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>

#include "asapo/unittests/MockLogger.h"
#include "asapo/common/error.h"

#include "asapo/request/request_pool.h"
#include "asapo/request/request_pool_error.h"
#include "asapo/request/request_handler_factory.h"
#include "mocking.h"

#include "asapo/io/io_factory.h"

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
    TestRequest(GenericRequestHeader header, uint64_t timeout): GenericRequest(header, timeout) {};
};


class RequestPoolTests : public testing::Test {
  public:
    NiceMock<MockRequestHandler>* mock_request_handler = new testing::NiceMock<MockRequestHandler>;
    NiceMock<asapo::MockLogger> mock_logger;
    MockRequestHandlerFactory request_handler_factory{mock_request_handler};
    const uint8_t nthreads = 1;
    asapo::RequestPool pool {nthreads, &request_handler_factory, &mock_logger};
    std::unique_ptr<GenericRequest> request{new TestRequest{GenericRequestHeader{asapo::kOpcodeUnknownOp,0,1000000}, 0}};
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

    asapo::RequestPool pool{4, &factory, &mock_logger};
}

TEST_F(RequestPoolTests, AddRequestDoesNotGoFurtherWhenNotReady) {

    EXPECT_CALL(*mock_request_handler, ReadyProcessRequest()).Times(AtLeast(1)).WillRepeatedly(Return(false));
    EXPECT_CALL(*mock_request_handler, PrepareProcessingRequestLocked()).Times(0);

    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RequestPoolTests, NRequestsInPoolInitial) {
    auto nreq = pool.NRequestsInPool();
    ASSERT_THAT(nreq, Eq(0));
}

TEST_F(RequestPoolTests, TimeOut) {
    std::unique_ptr<GenericRequest> request{new TestRequest{GenericRequestHeader{}, 10}};
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_CALL(*mock_request_handler, ReadyProcessRequest()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(*mock_request_handler, PrepareProcessingRequestLocked()).Times(0);
    EXPECT_CALL(*mock_request_handler, ProcessRequestUnlocked_t(_, _)).Times(0);
    EXPECT_CALL(*mock_request_handler, ProcessRequestTimeout(_)).Times(1);

    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_THAT(err, Eq(nullptr));
}

void ExpectSend(MockRequestHandler* mock_handler, int ntimes = 1) {
    EXPECT_CALL(*mock_handler, ReadyProcessRequest()).Times(ntimes).WillRepeatedly(Return(true));
    EXPECT_CALL(*mock_handler, PrepareProcessingRequestLocked()).Times(ntimes);
    EXPECT_CALL(*mock_handler, ProcessRequestUnlocked_t(_, _)).Times(ntimes).WillRepeatedly(
        DoAll(            testing::SetArgPointee<1>(false),
                          Return(true)
             ));
    EXPECT_CALL(*mock_handler, TearDownProcessingRequestLocked(true)).Times(ntimes);
}

void ExpectFailProcessRequest(MockRequestHandler* mock_handler) {
    EXPECT_CALL(*mock_handler, ReadyProcessRequest()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*mock_handler, PrepareProcessingRequestLocked()).Times(AtLeast(1));
    EXPECT_CALL(*mock_handler, ProcessRequestUnlocked_t(_, _)).Times(AtLeast(1)).WillRepeatedly(
        DoAll(            testing::SetArgPointee<1>(true),
                          Return(false)
             ));
    EXPECT_CALL(*mock_handler, TearDownProcessingRequestLocked(false)).Times(AtLeast(1));
}



TEST_F(RequestPoolTests, AddRequestIncreasesRetryCounter) {

    ExpectFailProcessRequest(mock_request_handler);

    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(mock_request_handler->retry_counter, Gt(0));
}


TEST_F(RequestPoolTests, AddRequestCallsSend) {

    ExpectSend(mock_request_handler);

    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RequestPoolTests, NRequestsInPool) {

    pool.AddRequest(std::move(request));
    auto nreq = pool.NRequestsInPool();

    ASSERT_THAT(nreq, Eq(1));
}

TEST_F(RequestPoolTests, NRequestsInPoolAccountsForRequestsInProgress) {
    ExpectSend(mock_request_handler, 1);

    pool.AddRequest(std::move(request));

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    auto nreq1 = pool.NRequestsInPool();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto nreq2 = pool.NRequestsInPool();

    ASSERT_THAT(nreq1, Eq(1));
    ASSERT_THAT(nreq2, Eq(0));
}


TEST_F(RequestPoolTests, AddRequestCallsSendTwoRequests) {

    TestRequest* request2 = new TestRequest{GenericRequestHeader{}, 0};

    ExpectSend(mock_request_handler, 2);

    auto err1 = pool.AddRequest(std::move(request));
    request.reset(request2);
    auto err2 = pool.AddRequest(std::move(request));

    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(err2, Eq(nullptr));
}

TEST_F(RequestPoolTests, RefuseAddRequestIfHitSizeLimitation) {
    TestRequest* request2 = new TestRequest{GenericRequestHeader{}, 0};

    pool.SetLimits(asapo::RequestPoolLimits({1,0}));
    pool.AddRequest(std::move(request));
    request.reset(request2);
    auto err = pool.AddRequest(std::move(request));

    auto nreq = pool.NRequestsInPool();

    ASSERT_THAT(nreq, Eq(1));
    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kNoSpaceLeft));
    auto err_data = static_cast<asapo::OriginalRequest*>(err->GetCustomData());
    ASSERT_THAT(err_data, Ne(nullptr));
}

TEST_F(RequestPoolTests, RefuseAddRequestIfHitMemoryLimitation) {
    auto header = GenericRequestHeader{};
    header.data_size = 100;
    TestRequest* request2 = new TestRequest{header, 0};

    pool.SetLimits(asapo::RequestPoolLimits({0,1}));
    pool.AddRequest(std::move(request));
    request.reset(request2);
    auto err = pool.AddRequest(std::move(request));

    auto nreq = pool.NRequestsInPool();

    ASSERT_THAT(nreq, Eq(1));
    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kNoSpaceLeft));
    auto err_data = static_cast<asapo::OriginalRequest*>(err->GetCustomData());
    ASSERT_THAT(err_data, Ne(nullptr));

}

TEST_F(RequestPoolTests, RefuseAddRequestsIfHitSizeLimitation) {

    TestRequest* request2 = new TestRequest{GenericRequestHeader{}, 0};

    std::vector<std::unique_ptr<GenericRequest>> requests;
    requests.push_back(std::move(request));
    requests.push_back(std::unique_ptr<GenericRequest> {request2});

    pool.SetLimits(asapo::RequestPoolLimits({1,0}));
    auto err = pool.AddRequests(std::move(requests));
    auto nreq = pool.NRequestsInPool();

    ASSERT_THAT(nreq, Eq(0));
    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kNoSpaceLeft));
}


TEST_F(RequestPoolTests, RefuseAddRequestsIfHitMemoryLimitation) {

    auto header = GenericRequestHeader{};
    header.data_size = 100;

    TestRequest* request2 = new TestRequest{header, 0};

    std::vector<std::unique_ptr<GenericRequest>> requests;
    requests.push_back(std::move(request));
    requests.push_back(std::unique_ptr<GenericRequest> {request2});

    pool.SetLimits(asapo::RequestPoolLimits({0,1}));
    auto err = pool.AddRequests(std::move(requests));
    auto nreq = pool.NRequestsInPool();

    ASSERT_THAT(nreq, Eq(0));
    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kNoSpaceLeft));
}


TEST_F(RequestPoolTests, AddRequestsOk) {

    TestRequest* request2 = new TestRequest{GenericRequestHeader{}, 0};

    ExpectSend(mock_request_handler, 2);

    std::vector<std::unique_ptr<GenericRequest>> requests;
    requests.push_back(std::move(request));
    requests.push_back(std::unique_ptr<GenericRequest> {request2});

    auto err = pool.AddRequests(std::move(requests));

    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RequestPoolTests, FinishProcessingThreads) {
    EXPECT_CALL(mock_logger, Debug(HasSubstr("finishing thread"))).Times(nthreads);
}

TEST_F(RequestPoolTests, WaitRequestsFinished) {
    ExpectSend(mock_request_handler);

    pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    auto err = pool.WaitRequestsFinished(1000);

    auto nreq = pool.NRequestsInPool();

    ASSERT_THAT(nreq, Eq(0));
    ASSERT_THAT(err, Eq(nullptr));

}

TEST_F(RequestPoolTests, WaitRequestsFinishedTimeout) {
    ExpectSend(mock_request_handler);

    pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    auto err = pool.WaitRequestsFinished(0);

    auto nreq = pool.NRequestsInPool();

    ASSERT_THAT(nreq, Eq(1));
    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kTimeout));

}

TEST_F(RequestPoolTests, StopThreads) {
    EXPECT_CALL(mock_logger, Debug(HasSubstr("finishing thread"))).Times(nthreads);

    pool.StopThreads();

    Mock::VerifyAndClearExpectations(&mock_logger);
}


}
