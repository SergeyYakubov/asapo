#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "asapo/unittests/MockLogger.h"
#include "../../src/receiver_data_server/receiver_data_server.h"
#include "../../src/receiver_data_server/net_server/rds_tcp_server.h"

#include "receiver_dataserver_mocking.h"

#include "asapo/common/io_error.h"
#include "../../src/receiver_data_server/receiver_data_server_error.h"
#include "../../src/statistics/statistics.h"

#include "../receiver_mocking.h"

using ::testing::Test;
using ::testing::Gt;
using ::testing::Ge;
using ::testing::Le;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Ref;
using ::testing::Return;
using ::testing::_;
using ::testing::SetArgPointee;
using ::testing::NiceMock;
using ::testing::HasSubstr;
using ::testing::DoAll;


using asapo::MockLogger;
using asapo::ReceiverDataServer;
using asapo::Error;
using asapo::GenericRequests;
using asapo::GenericRequest;
using asapo::ReceiverDataServerRequest;


namespace {

TEST(ReceiverDataServer, Constructor) {
    asapo::ReceiverDataServerConfig config;
    config.nthreads = 4;
    asapo::MockNetServer mock_net;
    ReceiverDataServer data_server{asapo::RdsNetServerPtr(&mock_net), asapo::LogLevel::Debug, nullptr, config};
    ASSERT_THAT(data_server.net__.release(), Eq(&mock_net));
    ASSERT_THAT(dynamic_cast<asapo::RequestPool*>(data_server.request_pool__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(data_server.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::Statistics*>(data_server.statistics__.get()), Ne(nullptr));
}

class ReceiverDataServerTests : public Test {
  public:
    asapo::ReceiverDataServerConfig config;
    std::string expected_address = "somehost:123";
    asapo::MockNetServer mock_net;
    asapo::MockPool mock_pool;
    NiceMock<asapo::MockLogger> mock_logger;
    NiceMock<asapo::MockStatistics> mock_statistics;

    ReceiverDataServer data_server{
        asapo::RdsNetServerPtr(&mock_net),
        asapo::LogLevel::Debug,
        nullptr,
        config};

    void SetUp() override {
        data_server.request_pool__ = std::unique_ptr<asapo::RequestPool> {&mock_pool};
        data_server.log__ = &mock_logger;
        data_server.statistics__ = std::unique_ptr<asapo::Statistics> {&mock_statistics};
    }
    void TearDown() override {
        data_server.net__.release();
        data_server.request_pool__.release();
        data_server.statistics__.release();
    }
};

TEST_F(ReceiverDataServerTests, TimeoutGetNewRequests) {
    EXPECT_CALL(mock_net, GetNewRequests_t(_)).WillOnce(
        DoAll(SetArgPointee<0>(asapo::IOErrorTemplates::kTimeout.Generate().release()),
              Return(std::vector<ReceiverDataServerRequest> {})
             )
    ).WillOnce(
        DoAll(SetArgPointee<0>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
              Return(std::vector<ReceiverDataServerRequest> {})
             )
    );

    EXPECT_CALL(mock_pool, AddRequests_t(_)).Times(0);
    EXPECT_CALL(mock_statistics, SendIfNeeded_t(false)).Times(2);

    data_server.Run();
}



TEST_F(ReceiverDataServerTests, ErrorGetNewRequests) {
    EXPECT_CALL(mock_net, GetNewRequests_t(_)).WillOnce(
        DoAll(SetArgPointee<0>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
              Return(std::vector<ReceiverDataServerRequest> {})
             )
    );

    auto errtext = asapo::IOErrorTemplates::kUnknownIOError.Generate()->Explain();

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("stopped"), HasSubstr(errtext))));

    data_server.Run();
}

TEST_F(ReceiverDataServerTests, ErrorAddingRequests) {
    EXPECT_CALL(mock_net, GetNewRequests_t(_)).WillOnce(
        DoAll(SetArgPointee<0>(nullptr),
              Return(std::vector<ReceiverDataServerRequest> {})
             )
    );

    EXPECT_CALL(mock_pool, AddRequests_t(_)).WillOnce(
        Return(asapo::ReceiverDataServerErrorTemplates::kMemoryPool.Generate("cannot add request to pool").release())
    );

    auto errtext = asapo::ReceiverDataServerErrorTemplates::kMemoryPool.Generate("cannot add request to pool")->Explain();

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("stopped"), HasSubstr(errtext))));

    data_server.Run();
}

TEST_F(ReceiverDataServerTests, Ok) {
    EXPECT_CALL(mock_net, GetNewRequests_t(_)).WillOnce(
        DoAll(SetArgPointee<0>(nullptr),
              Return(std::vector<ReceiverDataServerRequest> {})
             )
    ).WillOnce(
        DoAll(SetArgPointee<0>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
              Return(std::vector<ReceiverDataServerRequest> {})
             )
    );

    EXPECT_CALL(mock_pool, AddRequests_t(_)).WillOnce(
        Return(nullptr)
    );

    data_server.Run();
}

}
