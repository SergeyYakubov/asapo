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
using ::testing::Contains;
using ::testing::IsEmpty;

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
    asapo::SocketDescriptor expected_master_socket = 1;
    asapo::ListSocketDescriptors expected_client_sockets{2, 3, 4};
    std::vector<std::string> expected_new_connections = {"test1", "test2"};
    void SetUp() override {
        tcp_server.io__ = std::unique_ptr<asapo::IO> {&mock_io};
        tcp_server.log__ = &mock_logger;
    }
    void TearDown() override {
        tcp_server.io__.release();
    }
    void ExpectListenMaster(bool ok);
    void WaitSockets(bool ok);
    void MockReceiveRequest(bool ok );
};

void TCPServerTests::ExpectListenMaster(bool ok) {
    EXPECT_CALL(mock_io, CreateAndBindIPTCPSocketListener_t(expected_address, asapo::kMaxPendingConnections, _))
    .WillOnce(DoAll(
                  SetArgPointee<2>(ok ? nullptr : asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(expected_master_socket)
              ));
}

void TCPServerTests::WaitSockets(bool ok) {
    EXPECT_CALL(mock_io, WaitSocketsActivity_t(expected_master_socket, _, _, _)).WillOnce(DoAll(
                SetArgPointee<2>(expected_new_connections),
                SetArgPointee<3>(ok ? nullptr : asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
                Return(ok ? expected_client_sockets : asapo::ListSocketDescriptors{})
            ));
    if (ok) {
        for (auto conn : expected_new_connections) {
            EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("connection"),
                                                 HasSubstr(conn)
                                                )
                                          )
                       );

        }
    }
}


TEST_F(TCPServerTests, GetNewRequestsInitializesSocket) {
    Error err;
    ExpectListenMaster(false);

    auto requests = tcp_server.GetNewRequests(&err);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(requests, IsEmpty());
}

TEST_F(TCPServerTests, GetNewRequestsInitializesSocketOnlyOnce) {
    Error err;
    ExpectListenMaster(false);

    tcp_server.GetNewRequests(&err);
    tcp_server.GetNewRequests(&err);

//    ASSERT_THAT(err, Ne(nullptr));
}

void TCPServerTests::MockReceiveRequest(bool ok ) {
    for (auto conn : expected_client_sockets) {
        EXPECT_CALL(mock_io, Receive_t(conn, _, _, _))
        .WillOnce(
            DoAll(SetArgPointee<3>(ok ? nullptr : asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(0))
        );
        if (!ok) {
            std::string connected_uri = std::to_string(conn);
            EXPECT_CALL(mock_io, AddressFromSocket_t(conn)).WillOnce(Return(connected_uri));
            EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("request"), HasSubstr(connected_uri))));
        }
    }
}


TEST_F(TCPServerTests, GetNewRequestsWaitsSocketActivitiesError) {
    Error err;
    ExpectListenMaster(true);
    WaitSockets(false);

    auto requests = tcp_server.GetNewRequests(&err);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(requests, IsEmpty());
}

TEST_F(TCPServerTests, GetNewRequestsWaitsSocketReceiveFailure) {
    Error err;
    ExpectListenMaster(true);
    WaitSockets(true);
    MockReceiveRequest(false);

    auto requests = tcp_server.GetNewRequests(&err);

    ASSERT_THAT(err, Ne(nullptr));

//    ASSERT_THAT(requests, IsEmpty());
}


}
