#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "unittests/MockLogger.h"
#include "../../src/receiver_data_server/receiver_data_server.h"
#include "../../src/receiver_data_server/tcp_server.h"

#include "receiver_dataserver_mocking.h"

#include "common/io_error.h"
#include "../../src/receiver_data_server/receiver_data_server_error.h"


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


using asapo::MockLogger;
using asapo::ReceiverDataServer;
using asapo::Error;
using asapo::GenericRequests;
using asapo::GenericRequest;
using asapo::ReceiverDataServerRequest;


namespace {

TEST(ReceiverDataServer, Constructor) {
    ReceiverDataServer data_server{"", asapo::LogLevel::Debug, 4, nullptr};
    ASSERT_THAT(dynamic_cast<const asapo::TcpServer*>(data_server.net__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::RequestPool*>(data_server.request_pool__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(data_server.log__), Ne(nullptr));
}

class ReceiverDataServerTests : public Test {
  public:
    std::string expected_address = "somehost:123";
    ReceiverDataServer data_server{expected_address, asapo::LogLevel::Debug, 0, nullptr};
    asapo::MockNetServer mock_net;
    asapo::MockPool mock_pool;
    NiceMock<asapo::MockLogger> mock_logger;
    void SetUp() override {
        data_server.net__ = std::unique_ptr<asapo::NetServer> {&mock_net};
        data_server.request_pool__ = std::unique_ptr<asapo::RequestPool> {&mock_pool};
        data_server.log__ = &mock_logger;
    }
    void TearDown() override {
        data_server.net__.release();
        data_server.request_pool__.release();
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
