#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>

#include "unittests/MockLogger.h"
#include "common/error.h"

#include "../src/request_handler_tcp.h"
#include "../src/request_pool.h"
#include "../src/receiver_discovery_service.h"

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

using asapo::ReceiversList;
using asapo::RequestHandlerTcp;
using asapo::RequestPool;
using asapo::Error;
using asapo::GenericNetworkRequestHeader;

const std::string expected_endpoint{"endpoint"};



class MockRequest : public RequestHandlerTcp {
  public:
    MockRequest() : RequestHandlerTcp(asapo::GenericNetworkRequestHeader{}, nullptr, nullptr) {};
    Error Handle(asapo::SocketDescriptor* sd, const ReceiversList &receivers_list, bool rebalance) override {
        return Error {Send_t(sd, receivers_list, rebalance)};
    }

    MOCK_METHOD3(Send_t, asapo::SimpleError * (asapo::SocketDescriptor*, const ReceiversList&, bool));
};

class RequestPoolTests : public testing::Test {
  public:
    NiceMock<asapo::MockLogger> mock_logger;
    const uint8_t nthreads = 4;
    const uint64_t max_size = 1024 * 1024 * 1024;
    NiceMock<MockDiscoveryService> mock_discovery;
    asapo::RequestPool pool {nthreads, max_size, &mock_discovery};
    std::unique_ptr<RequestHandlerTcp> request;
    NiceMock<MockRequest>* mock_request = new testing::NiceMock<MockRequest>;
    ReceiversList expected_receivers_list1{"ip1", "ip2", "ip3"};
    ReceiversList expected_receivers_list2{"ip4", "ip5", "ip6"};
    void SetUp() override {
        pool.log__ = &mock_logger;
        request.reset(mock_request);
        ON_CALL(mock_discovery, MaxConnections()).WillByDefault(Return(100));
        ON_CALL(mock_discovery, RotatedUriList(_)).WillByDefault(Return(expected_receivers_list1));

    }
    void TearDown() override {
    }
};

void ExpectSend(MockRequest* request, const ReceiversList& list, bool connected = false, bool rebalance = false) {
    auto descriptor = connected ? 1 : asapo::kDisconnectedSocketDescriptor;
    EXPECT_CALL(*request, Send_t(testing::Pointee(descriptor), list, rebalance))
    .WillOnce(DoAll(
                  testing::SetArgPointee<0>(1),
                  Return(nullptr)
              )
             );
}

TEST(RequestPool, Constructor) {
    NiceMock<MockDiscoveryService> mock_discovery;
    EXPECT_CALL(mock_discovery, StartCollectingData());

    asapo::RequestPool pool{4, 4, &mock_discovery};

    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(pool.log__), Ne(nullptr));
}

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

    MockRequest* mock_request2 = new MockRequest;

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

    MockRequest* mock_request2 = new MockRequest;


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



}
