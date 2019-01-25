#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "unittests/MockLogger.h"
#include "unittests/MockIO.h"

#include "../../src/receiver_data_server/tcp_server.h"



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


using asapo::TcpServer;
using asapo::MockIO;
using asapo::Error;


namespace {

TEST(TCPServer, Constructor) {
    TcpServer tcp_server("");
    ASSERT_THAT(dynamic_cast<asapo::IO*>(tcp_server.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(tcp_server.log__), Ne(nullptr));

}

class TCPServerTests : public Test {
  public:
    std::string expected_address = "somehost:123";
    TcpServer tcp_server{expected_address};
    NiceMock<MockIO> mock_io;
    NiceMock<asapo::MockLogger> mock_logger;
    asapo::SocketDescriptor expected_socket = 1;
    void SetUp() override {
        tcp_server.io__ = std::unique_ptr<asapo::IO> {&mock_io};
        tcp_server.log__ = &mock_logger;
    }
    void TearDown() override {
        tcp_server.io__.release();
    }
    void ExpectListenMaster(bool ok);

};

void TCPServerTests::ExpectListenMaster(bool ok) {
    EXPECT_CALL(mock_io, CreateAndBindIPTCPSocketListener_t(expected_address, asapo::kMaxPendingConnections, _))
    .WillOnce(DoAll(
                  SetArgPointee<2>(ok ? nullptr : asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(expected_socket)
              ));

}

TEST_F(TCPServerTests, GetNewRequestsInitializesSocket) {
    Error err;
    ExpectListenMaster(false);
    tcp_server.GetNewRequests(&err);
    ASSERT_THAT(err, Ne(nullptr));
}

TEST_F(TCPServerTests, GetNewRequestsInitializesSocketOnlyOnce) {
    Error err;
    ExpectListenMaster(false);
    tcp_server.GetNewRequests(&err);
    tcp_server.GetNewRequests(&err);
//    ASSERT_THAT(err, Ne(nullptr));
}



}
