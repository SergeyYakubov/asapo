#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "unittests/MockLogger.h"
#include "unittests/MockIO.h"
#include "io/io_factory.h"
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
using ::testing::Mock;
using ::testing::DoAll;

using asapo::TcpServer;
using asapo::MockIO;
using asapo::Error;
using asapo::ListSocketDescriptors;
namespace {

TEST(TCPServer, Constructor) {
    TcpServer tcp_server("");
    ASSERT_THAT(dynamic_cast<asapo::IO*>(tcp_server.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(tcp_server.log__), Ne(nullptr));

}

std::string expected_address = "somehost:123";

class TCPServerTests : public Test {
  public:
    TcpServer tcp_server {expected_address};
    NiceMock<MockIO> mock_io;
    NiceMock<asapo::MockLogger> mock_logger;
    asapo::SocketDescriptor expected_master_socket = 1;
    ListSocketDescriptors expected_client_sockets{2, 3, 4};
    std::vector<std::string> expected_new_connections = {"test1", "test2"};
    void SetUp() override {
        tcp_server.io__ = std::unique_ptr<asapo::IO> {&mock_io};
        tcp_server.log__ = &mock_logger;
        for (auto conn : expected_client_sockets) {
            std::string connected_uri = std::to_string(conn);
            ON_CALL(mock_io, AddressFromSocket_t(conn)).WillByDefault(Return(connected_uri));
        }

    }
    void TearDown() override {
        tcp_server.io__.release();
    }
    void ExpectTcpBind(bool ok);
    void WaitSockets(bool ok, ListSocketDescriptors clients = {});
    void MockReceiveRequest(bool ok);
    void InitMasterServer();
    void ExpectReceiveOk();
    void ExpectReceiveRequestEof();
};

void TCPServerTests::ExpectTcpBind(bool ok) {
    EXPECT_CALL(mock_io, CreateAndBindIPTCPSocketListener_t(expected_address, asapo::kMaxPendingConnections, _))
    .WillOnce(DoAll(
                  SetArgPointee<2>(ok ? nullptr : asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(expected_master_socket)
              ));
}

void TCPServerTests::WaitSockets(bool ok, ListSocketDescriptors clients) {
    EXPECT_CALL(mock_io, WaitSocketsActivity_t(expected_master_socket, testing::Pointee(clients), _, _)).WillOnce(DoAll(
                SetArgPointee<2>(ok ? expected_new_connections : std::vector<std::string> {}),
                SetArgPointee<1>(expected_client_sockets),
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

void TCPServerTests::InitMasterServer() {
    ExpectTcpBind(true);
    ASSERT_THAT(tcp_server.Initialize(), Eq(nullptr));
}

TEST_F(TCPServerTests, Initialize_Error) {
    ExpectTcpBind(false);

    Error err = tcp_server.Initialize();

    ASSERT_THAT(err, Ne(nullptr));
}

TEST_F(TCPServerTests, Initialize_ErrorDoubleInitialize) {
    Error err;

    ExpectTcpBind(true);
    err = tcp_server.Initialize();
    ASSERT_THAT(err, Eq(nullptr));

    err = tcp_server.Initialize();
    ASSERT_THAT(err, Ne(nullptr));
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
            EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("request"), HasSubstr(connected_uri))));
        }
    }
}

void TCPServerTests::ExpectReceiveRequestEof() {
    for (auto conn : expected_client_sockets) {
        EXPECT_CALL(mock_io, Receive_t(conn, _, _, _))
        .WillOnce(
            DoAll(SetArgPointee<3>(asapo::ErrorTemplates::kEndOfFile.Generate().release()),
                  Return(0))
        );
        EXPECT_CALL(mock_io, CloseSocket_t(conn, _));

        std::string connected_uri = std::to_string(conn);
        EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("connection"), HasSubstr("closed"), HasSubstr(connected_uri))));
    }
}


ACTION_P2(A_ReceiveData, op_code, expected_id) {
    ((asapo::GenericRequestHeader*)arg1)->op_code = op_code;
    ((asapo::GenericRequestHeader*)arg1)->data_id = expected_id;
}


void TCPServerTests::ExpectReceiveOk() {
    for (auto conn : expected_client_sockets) {
        EXPECT_CALL(mock_io, Receive_t(conn, _, sizeof(asapo::GenericRequestHeader), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                A_ReceiveData(asapo::kOpcodeGetBufferData, conn),
                testing::ReturnArg<2>()
            ));
        EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("request"), HasSubstr("id: " + std::to_string(conn)),
                                             HasSubstr("opcode: " + std::to_string(asapo::kOpcodeGetBufferData)))));
    }
}

TEST_F(TCPServerTests, GetNewRequestsWaitsSocketActivitiesError) {
    Error err;
    InitMasterServer();
    WaitSockets(false);

    auto requests = tcp_server.GetNewRequests(&err);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(requests, IsEmpty());
}

TEST_F(TCPServerTests, GetNewRequestsWaitsSocketReceiveFailure) {
    Error err;
    InitMasterServer();
    WaitSockets(true);
    MockReceiveRequest(false);

    auto requests = tcp_server.GetNewRequests(&err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(requests, IsEmpty());

    Mock::VerifyAndClearExpectations(&mock_io);

    WaitSockets(false, expected_client_sockets);
    tcp_server.GetNewRequests(&err);

}

TEST_F(TCPServerTests, GetNewRequestsReadEof) {
    Error err;
    InitMasterServer();
    WaitSockets(true);
    ExpectReceiveRequestEof();

    auto requests = tcp_server.GetNewRequests(&err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(requests, IsEmpty());

    Mock::VerifyAndClearExpectations(&mock_io);

    WaitSockets(false, {});

    tcp_server.GetNewRequests(&err);

}

TEST_F(TCPServerTests, GetNewRequestsReadOk) {
    Error err;
    InitMasterServer();
    WaitSockets(true);
    ExpectReceiveOk();

    auto requests = tcp_server.GetNewRequests(&err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(requests.size(), Eq(3));
    int i = 0;
    for (auto conn : expected_client_sockets) {
        ASSERT_THAT(dynamic_cast<asapo::ReceiverDataServerRequest*>(requests[i].get()), Ne(nullptr));
        ASSERT_THAT(requests[i]->header.data_id, Eq(conn));
        ASSERT_THAT(requests[i]->header.op_code, Eq(asapo::kOpcodeGetBufferData));
//        ASSERT_THAT(requests[i]->source_id, Eq(conn));
//        ASSERT_THAT(requests[i]->server, Eq(&tcp_server));
        i++;
    }

}

TEST_F(TCPServerTests, SendResponse) {
    asapo::GenericNetworkResponse tmp {};
    asapo::ReceiverDataServerRequest expectedRequest {{}, 30};

    EXPECT_CALL(mock_io, Send_t(30, &tmp, sizeof(asapo::GenericNetworkResponse), _))
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
            Return(1)
        ));

    EXPECT_CALL(mock_logger, Error(HasSubstr("cannot send")));

    auto err = tcp_server.SendResponse(&expectedRequest, &tmp);

    ASSERT_THAT(err, Ne(nullptr));
}

TEST_F(TCPServerTests, SendResponseAndSlotData_SendResponseError) {
    asapo::GenericNetworkResponse tmp {};


    asapo::ReceiverDataServerRequest expectedRequest {{}, 30};
    asapo::CacheMeta expectedMeta {};
    expectedMeta.id = 20;
    expectedMeta.addr = (void*)0x9234;
    expectedMeta.size = 50;
    expectedMeta.lock = 123;

    EXPECT_CALL(mock_io, Send_t(30, &tmp, sizeof(asapo::GenericNetworkResponse), _))
    .WillOnce(DoAll(
                  testing::SetArgPointee<3>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(0)
              ));
    EXPECT_CALL(mock_logger, Error(HasSubstr("cannot send")));

    auto err = tcp_server.SendResponseAndSlotData(&expectedRequest, &tmp, &expectedMeta);

    ASSERT_THAT(err, Ne(nullptr));
}

TEST_F(TCPServerTests, SendResponseAndSlotData_SendDataError) {
    asapo::GenericNetworkResponse tmp {};

    asapo::ReceiverDataServerRequest expectedRequest {{}, 30};
    asapo::CacheMeta expectedMeta {};
    expectedMeta.id = 20;
    expectedMeta.addr = (void*)0x9234;
    expectedMeta.size = 50;
    expectedMeta.lock = 123;

    EXPECT_CALL(mock_io, Send_t(30, &tmp, sizeof(asapo::GenericNetworkResponse), _))
    .WillOnce(Return(1));
    EXPECT_CALL(mock_io, Send_t(30, expectedMeta.addr, expectedMeta.size, _))
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
            Return(0)
        ));

    EXPECT_CALL(mock_logger, Error(HasSubstr("cannot send")));

    auto err = tcp_server.SendResponseAndSlotData(&expectedRequest, &tmp, &expectedMeta);

    ASSERT_THAT(err, Ne(nullptr));
}

TEST_F(TCPServerTests, SendResponseAndSlotData_Ok) {
    asapo::GenericNetworkResponse tmp {};

    asapo::ReceiverDataServerRequest expectedRequest {{}, 30};
    asapo::CacheMeta expectedMeta {};
    expectedMeta.id = 20;
    expectedMeta.addr = (void*)0x9234;
    expectedMeta.size = 50;
    expectedMeta.lock = 123;

    EXPECT_CALL(mock_io, Send_t(30, &tmp, sizeof(asapo::GenericNetworkResponse), _))
    .WillOnce(Return(1));
    EXPECT_CALL(mock_io, Send_t(30, expectedMeta.addr, expectedMeta.size, _))
    .WillOnce(Return(expectedMeta.size));

    auto err = tcp_server.SendResponseAndSlotData(&expectedRequest, &tmp, &expectedMeta);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(TCPServerTests, HandleAfterError) {
    EXPECT_CALL(mock_io, CloseSocket_t(expected_client_sockets[0], _));
    tcp_server.HandleAfterError(expected_client_sockets[0]);
}

}
