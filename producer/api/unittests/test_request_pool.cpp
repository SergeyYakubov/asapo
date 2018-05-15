#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>

#include "unittests/MockLogger.h"
#include "common/error.h"

#include "../src/request.h"
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

using ::testing::InSequence;
using ::testing::HasSubstr;

using asapo::Request;
using asapo::RequestPool;
using asapo::Error;

const std::string expected_endpoint{"endpoint"};

class MockDiscoveryService : public asapo::ReceiverDiscoveryService {
  public:
    MockDiscoveryService() : ReceiverDiscoveryService{expected_endpoint, 1} {};
    MOCK_METHOD0(StartCollectingData, void());
    MOCK_METHOD0(MaxConnections, uint64_t());
    MOCK_METHOD1(RotatedUriList, asapo::ReceiversList(uint64_t));
};


class MockRequest : public Request {
  public:
    std::unique_ptr<asapo::IO>   io = std::unique_ptr<asapo::IO> {asapo::GenerateDefaultIO()};
    MockRequest() : Request(io.get(), asapo::GenericNetworkRequestHeader{}, nullptr, nullptr) {};
    Error Send(asapo::SocketDescriptor* sd, const asapo::ReceiversList& receivers_list, bool rebalance) override {
        return Error {Send_t(sd, receivers_list,rebalance)};
    }

    MOCK_METHOD3(Send_t, asapo::SimpleError * (asapo::SocketDescriptor*, const asapo::ReceiversList&,bool));
};

class RequestPoolTests : public testing::Test {
  public:
    testing::NiceMock<asapo::MockLogger> mock_logger;
    const uint8_t nthreads = 4;
    const uint64_t max_size = 1024 * 1024 * 1024;
    testing::NiceMock<MockDiscoveryService> mock_discovery;
    asapo::RequestPool pool {nthreads, max_size, &mock_discovery};
    std::unique_ptr<Request> request;
    MockRequest* mock_request = new MockRequest;
    asapo::ReceiversList expected_receivers_list{"ip1", "ip2", "ip3"};
    void SetUp() override {
        pool.log__ = &mock_logger;
        request.reset(mock_request);
        ON_CALL(mock_discovery, MaxConnections()).WillByDefault(Return(100));
        ON_CALL(mock_discovery, RotatedUriList(_)).WillByDefault(Return(expected_receivers_list));

    }
    void TearDown() override {
    }
};

TEST(RequestPool, Constructor) {
    auto  io = std::unique_ptr<asapo::IO> {asapo::GenerateDefaultIO()};
    testing::NiceMock<MockDiscoveryService> mock_discovery;

    EXPECT_CALL(mock_discovery, StartCollectingData());

    asapo::RequestPool pool{4, 4, &mock_discovery};

    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(pool.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::ReceiverDiscoveryService*>(pool.discovery_service__), Ne(nullptr));
}

TEST(RequestPool, AddRequestFailsDueToSize) {
    auto  io = std::unique_ptr<asapo::IO> {asapo::GenerateDefaultIO()};
    asapo::ReceiverDiscoveryService discovery{expected_endpoint, 1000};
    RequestPool pool{4, 0, &discovery};
    asapo::GenericNetworkRequestHeader header{asapo::Opcode::kNetOpcodeCount, 1, 1};
    std::unique_ptr<Request> request{new Request{io.get(), header, nullptr, [](asapo::GenericNetworkRequestHeader, asapo::Error) {}}};
    auto err = pool.AddRequest(std::move(request));
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kRequestPoolIsFull));

}

TEST_F(RequestPoolTests, AddRequestCallsSend) {

    EXPECT_CALL(mock_discovery, RotatedUriList(_)).WillOnce(Return(asapo::ReceiversList{"ip3", "ip2", "ip1"}));

    EXPECT_CALL(*mock_request, Send_t(testing::Pointee(asapo::kDisconnectedSocketDescriptor),
                                      testing::ElementsAre("ip3", "ip2", "ip1"),false)).
    WillOnce(
        Return(nullptr)
    );

    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RequestPoolTests, AddRequestCallsSendTwice) {
    asapo::SimpleError* send_error = new asapo::SimpleError("www");

    EXPECT_CALL(*mock_request, Send_t(testing::Pointee(asapo::kDisconnectedSocketDescriptor), testing::ElementsAre("ip1",
                                      "ip2", "ip3"),false))
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

    EXPECT_CALL(*mock_request, Send_t(testing::Pointee(asapo::kDisconnectedSocketDescriptor), testing::ElementsAre("ip1",
                                      "ip2", "ip3"),false))
    .WillOnce(DoAll(
                  testing::SetArgPointee<0>(1),
                  Return(nullptr)
              )
             );
    EXPECT_CALL(*mock_request2, Send_t(testing::Pointee(1), testing::ElementsAre("ip1", "ip2", "ip3"),false))
    .WillOnce(DoAll(
                  testing::SetArgPointee<0>(1),
                  Return(nullptr)
              )
             );
    auto err1 = pool.AddRequest(std::move(request));
    request.reset(mock_request2);
    auto err2 = pool.AddRequest(std::move(request));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(err2, Eq(nullptr));
}



TEST_F(RequestPoolTests, AddRequestDoesNotCallsSendWhenNoConnactionsAllowed) {
    EXPECT_CALL(*mock_request, Send_t(_, _,_)).Times(0);

    EXPECT_CALL(mock_discovery, MaxConnections()).WillRepeatedly(Return(0));

    pool.discovery_service__ = &mock_discovery;
    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_THAT(err, Eq(nullptr));
}



TEST_F(RequestPoolTests, FinishProcessingThreads) {
    EXPECT_CALL(mock_logger, Debug(HasSubstr("finishing thread"))).Times(nthreads);
}


TEST_F(RequestPoolTests, Rebalance) {
    EXPECT_CALL(mock_discovery, MaxConnections()).WillRepeatedly(Return(1));

    MockRequest* mock_request2 = new MockRequest;

    EXPECT_CALL(mock_discovery, RotatedUriList(_)).Times(2).
    WillOnce(Return(asapo::ReceiversList{"ip3", "ip2", "ip1"})).
    WillOnce(Return(asapo::ReceiversList{"ip4", "ip5", "ip6"}));


    EXPECT_CALL(*mock_request, Send_t(testing::Pointee(asapo::kDisconnectedSocketDescriptor), testing::ElementsAre("ip3",
                                      "ip2", "ip1"),false))
    .WillOnce(DoAll(
                  testing::SetArgPointee<0>(1),
                  Return(nullptr)
              )
             );

    EXPECT_CALL(*mock_request2, Send_t(testing::Pointee(1), testing::ElementsAre("ip4", "ip5", "ip6"),true))
    .WillOnce(DoAll(
                  testing::SetArgPointee<0>(1),
                  Return(nullptr)
              )
             );

    auto err1 = pool.AddRequest(std::move(request));
    request.reset(mock_request2);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto err2 = pool.AddRequest(std::move(request));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(err2, Eq(nullptr));

}



}
