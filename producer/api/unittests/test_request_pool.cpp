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

using asapo::RequestHandler;
using asapo::RequestPool;
using asapo::Error;
using asapo::ErrorInterface;
using asapo::Request;
using asapo::GenericNetworkRequestHeader;


MockRequestHandler mock_request_handler;


class MockRequestHandlerFactory : public asapo::RequestHandlerFactory {
 public:
  MockRequestHandlerFactory():RequestHandlerFactory(asapo::RequestHandlerType::kTcp, nullptr){};
  std::unique_ptr<RequestHandler> NewRequestHandler(uint64_t thread_id) override {
      return std::unique_ptr<RequestHandler>{&mock_request_handler};
  }
};



class RequestPoolTests : public testing::Test {
  public:
    NiceMock<asapo::MockLogger> mock_logger;
    MockRequestHandlerFactory request_handler_factory;
    const uint8_t nthreads = 4;
    const uint64_t max_size = 1024 * 1024 * 1024;
    asapo::RequestPool pool {nthreads, max_size, &request_handler_factory};
    std::unique_ptr<Request> request;
    NiceMock<MockRequestHandler>* mock_request_handler = new testing::NiceMock<MockRequestHandler>;
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

/*

TEST(RequestPool, AddRequestFailsDueToSize) {
    asapo::ReceiverDiscoveryService discovery{expected_endpoint, 1000};
    RequestPool pool{4, 0, &discovery};
    std::unique_ptr<RequestHandlerTcp> request{new RequestHandlerTcp{GenericNetworkRequestHeader{}, nullptr, nullptr}};

    auto err = pool.AddRequest(std::move(request));

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kRequestPoolIsFull));

}

TEST_F(RequestPoolTests, AddRequestCallsSend) {
    EXPECT_CALL(mock_discovery, RotatedUriList(_)).WillOnce(Return(expected_receivers_list1));
    ExpectSend(mock_request, expected_receivers_list1);

    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RequestPoolTests, AddRequestCallsSendTwice) {
    asapo::SimpleError* send_error = new asapo::SimpleError("");

    EXPECT_CALL(*mock_request, Send_t(testing::Pointee(asapo::kDisconnectedSocketDescriptor), expected_receivers_list1,
                                      false))
    .Times(2)
    .WillOnce(DoAll(
                  testing::SetArgPointee<0>(asapo::kDisconnectedSocketDescriptor),
                  Return(send_error)
              ))
    .WillOnce(DoAll(
                  testing::SetArgPointee<0>(1),
                  Return(nullptr)
              ));

    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RequestPoolTests, AddRequestCallsSendTwoRequests) {
    EXPECT_CALL(mock_discovery, MaxConnections()).WillRepeatedly(Return(1));

    MockRequestHandler* mock_request2 = new MockRequestHandler;

    ExpectSend(mock_request, expected_receivers_list1);
    ExpectSend(mock_request2, expected_receivers_list1, true);


    auto err1 = pool.AddRequest(std::move(request));
    request.reset(mock_request2);
    auto err2 = pool.AddRequest(std::move(request));

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(err2, Eq(nullptr));
}



TEST_F(RequestPoolTests, AddRequestDoesNotCallsSendWhenNoConnactionsAllowed) {
    EXPECT_CALL(mock_discovery, MaxConnections()).WillRepeatedly(Return(0));
    EXPECT_CALL(*mock_request, Send_t(_, _, _)).Times(0);

    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RequestPoolTests, NRequestsInQueue) {
    auto nreq = pool.NRequestsInQueue();
    ASSERT_THAT(nreq, Eq(0));
}


TEST_F(RequestPoolTests, FinishProcessingThreads) {
    EXPECT_CALL(mock_logger, Debug(HasSubstr("finishing thread"))).Times(nthreads);
}


TEST_F(RequestPoolTests, Rebalance) {
    EXPECT_CALL(mock_discovery, MaxConnections()).WillRepeatedly(Return(1));

    MockRequestHandler* mock_request2 = new MockRequestHandler;


    EXPECT_CALL(mock_discovery, RotatedUriList(_)).Times(2).
    WillOnce(Return(expected_receivers_list1)).
    WillOnce(Return(expected_receivers_list2));


    ExpectSend(mock_request, expected_receivers_list1);
    ExpectSend(mock_request2, expected_receivers_list2, true, true);


    auto err1 = pool.AddRequest(std::move(request));
    request.reset(mock_request2);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto err2 = pool.AddRequest(std::move(request));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(err2, Eq(nullptr));

}

*/

}
