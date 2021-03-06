#include <gmock/gmock.h>
#include "gtest/gtest.h"

#include "asapo/io/io.h"
#include "asapo/unittests/MockIO.h"
#include "mocking.h"
#include "../src/tcp_consumer_client.h"
#include "../../../../common/cpp/src/system_io/system_io.h"
#include "asapo/common/networking.h"

using asapo::IO;
using asapo::MessageMeta;
using asapo::MessageData;
using asapo::MockIO;
using asapo::TcpConsumerClient;
using asapo::MockTCPConnectionPool;


using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Ne;
using ::testing::Test;
using ::testing::_;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetArgReferee;
using testing::AllOf;
using ::testing::DoAll;

namespace {

TEST(TcpClient, Constructor) {
    auto client = std::unique_ptr<TcpConsumerClient> {new TcpConsumerClient()};
    ASSERT_THAT(dynamic_cast<asapo::SystemIO*>(client->io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::TcpConnectionPool*>(client->connection_pool__.get()), Ne(nullptr));
}

MATCHER_P4(M_CheckSendRequest, op_code, buf_id, data_size, message,
           "Checks if a valid GenericRequestHeader was Send") {
    return ((asapo::GenericRequestHeader*) arg)->op_code == op_code
           && ((asapo::GenericRequestHeader*) arg)->data_id == uint64_t(buf_id)
           && ((asapo::GenericRequestHeader*) arg)->data_size == uint64_t(data_size)
           && strcmp(((asapo::GenericRequestHeader*) arg)->api_version, "v0.1") == 0
           && strcmp(((asapo::GenericRequestHeader*) arg)->message, message) == 0;
}

ACTION_P(A_WriteSendResponse, error_code) {
    ((asapo::SendResponse*)arg1)->op_code = asapo::kOpcodeGetBufferData;
    ((asapo::SendResponse*)arg1)->error_code = error_code;
}


class TcpClientTests : public Test {
  public:
    std::unique_ptr<TcpConsumerClient> client = std::unique_ptr<TcpConsumerClient> {new TcpConsumerClient()};
    NiceMock<MockIO> mock_io;
    NiceMock<MockTCPConnectionPool> mock_connection_pool;
    MessageMeta info;
    std::string expected_uri = "test:8400";
    uint64_t expected_buf_id = 123;
    size_t expected_size = 1233;
    MessageData data;
    asapo::SocketDescriptor expected_sd = 1;
    void SetUp() override {
        info.source = expected_uri;
        info.buf_id = expected_buf_id;
        info.size = expected_size;
        client->io__ = std::unique_ptr<IO> {&mock_io};
        client->connection_pool__ = std::unique_ptr<asapo::TcpConnectionPool> {&mock_connection_pool};
    }
    void TearDown() override {
        client->io__.release();
        client->connection_pool__.release();
    }

    void ExpectNewConnection(bool reused, bool ok) {
        EXPECT_CALL(mock_connection_pool, GetFreeConnection_t(expected_uri, _, _)).WillOnce(
            DoAll(
                SetArgPointee<1>(reused),
                SetArgPointee<2>(ok ? nullptr : asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
                Return(ok ? expected_sd : asapo::kDisconnectedSocketDescriptor))
        );
    }

    void ExpectReconnect(bool ok) {
        EXPECT_CALL(mock_connection_pool, Reconnect_t(expected_sd, _)).WillOnce(
            DoAll(
                SetArgPointee<1>(ok ? nullptr : asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
                Return(ok ? expected_sd + 1 : asapo::kDisconnectedSocketDescriptor))
        );
    }

    void ExpectSendRequest(asapo::SocketDescriptor sd, bool ok = true) {
        EXPECT_CALL(mock_io, Send_t(sd, M_CheckSendRequest(asapo::kOpcodeGetBufferData, expected_buf_id,
                                    expected_size, ""),
                                    sizeof(asapo::GenericRequestHeader), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(ok ? nullptr
                                          : asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
                Return(ok ? 1 : 0)
            ));
        if (!ok) {
            EXPECT_CALL(mock_io, CloseSocket_t(sd, _));
            EXPECT_CALL(mock_connection_pool, ReleaseConnection(sd));
        }
    }



    void ExpectGetResponce(asapo::SocketDescriptor sd, bool ok, asapo::NetworkErrorCode responce_code) {

        EXPECT_CALL(mock_io, Receive_t(sd, _, sizeof(asapo::SendResponse), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(ok ? nullptr : asapo::IOErrorTemplates::kConnectionRefused.Generate().release()),
                A_WriteSendResponse(responce_code),
                testing::ReturnArg<2>()
            ));
        if (!ok) {
            EXPECT_CALL(mock_io, CloseSocket_t(sd, _));
            EXPECT_CALL(mock_connection_pool, ReleaseConnection(sd));
        }

    }

    void ExpectGetData(asapo::SocketDescriptor sd, bool ok) {

        EXPECT_CALL(mock_io, Receive_t(sd, _, (size_t) expected_size, _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(ok ? nullptr : asapo::IOErrorTemplates::kTimeout.Generate().release()),
                testing::Return(ok ? expected_size : 0)
            ));
        if (!ok) {
            EXPECT_CALL(mock_io, CloseSocket_t(sd, _));
        }
        EXPECT_CALL(mock_connection_pool, ReleaseConnection(sd));
    }
};

TEST_F(TcpClientTests, ErrorGetNewConnection) {
    ExpectNewConnection(false, false);

    auto err = client->GetData(&info, &data);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}

TEST_F(TcpClientTests, SendHeaderForNewConnectionReturnsError) {
    ExpectNewConnection(false, true);
    ExpectSendRequest(expected_sd, false);

    auto err = client->GetData(&info, &data);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kBadFileNumber));
}

TEST_F(TcpClientTests, OnErrorSendHeaderTriesToReconnectAndFails) {
    ExpectNewConnection(true, true);
    ExpectSendRequest(expected_sd, false);
    ExpectReconnect(false);

    auto err = client->GetData(&info, &data);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}

TEST_F(TcpClientTests, OnErrorSendHeaderTriesToReconnectAndSendsAnotherRequest) {
    ExpectNewConnection(true, true);
    ExpectSendRequest(expected_sd, false);
    ExpectReconnect(true);
    ExpectSendRequest(expected_sd + 1, false);

    auto err = client->GetData(&info, &data);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kBadFileNumber));
}

TEST_F(TcpClientTests, GetResponceReturnsError) {
    ExpectNewConnection(false, true);
    ExpectSendRequest(expected_sd, true);
    ExpectGetResponce(expected_sd, false, asapo::kNetErrorNoError);

    auto err = client->GetData(&info, &data);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kConnectionRefused));
}

TEST_F(TcpClientTests, GetResponceReturnsNoData) {
    ExpectNewConnection(false, true);
    ExpectSendRequest(expected_sd, true);
    ExpectGetResponce(expected_sd, true, asapo::kNetErrorNoData);
    EXPECT_CALL(mock_connection_pool, ReleaseConnection(expected_sd));

    auto err = client->GetData(&info, &data);

    ASSERT_THAT(err, Ne(nullptr));
}

TEST_F(TcpClientTests, GetResponceReturnsWrongRequest) {
    ExpectNewConnection(false, true);
    ExpectSendRequest(expected_sd, true);
    ExpectGetResponce(expected_sd, true, asapo::kNetErrorWrongRequest);
    EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));

    auto err = client->GetData(&info, &data);

    ASSERT_THAT(err, Ne(nullptr));
}

TEST_F(TcpClientTests, GetResponceReturnsUnsupported) {
    ExpectNewConnection(false, true);
    ExpectSendRequest(expected_sd, true);
    ExpectGetResponce(expected_sd, true, asapo::kNetErrorNotSupported);
    EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
    EXPECT_CALL(mock_connection_pool, ReleaseConnection(expected_sd));

    auto err = client->GetData(&info, &data);

    ASSERT_THAT(err, Ne(nullptr));
}


TEST_F(TcpClientTests, ErrorGettingData) {
    ExpectNewConnection(false, true);
    ExpectSendRequest(expected_sd, true);
    ExpectGetResponce(expected_sd, true, asapo::kNetErrorNoError);
    ExpectGetData(expected_sd, false);

    auto err = client->GetData(&info, &data);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kTimeout));
}

TEST_F(TcpClientTests, OkGettingData) {
    ExpectNewConnection(false, true);
    ExpectSendRequest(expected_sd, true);
    ExpectGetResponce(expected_sd, true, asapo::kNetErrorNoError);
    ExpectGetData(expected_sd, true);

    auto err = client->GetData(&info, &data);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(TcpClientTests, OkGettingDataWithReconnect) {
    ExpectNewConnection(true, true);
    ExpectSendRequest(expected_sd, false);
    ExpectReconnect(true);
    ExpectSendRequest(expected_sd + 1, true);
    ExpectGetResponce(expected_sd + 1, true, asapo::kNetErrorNoError);
    ExpectGetData(expected_sd + 1, true);

    auto err = client->GetData(&info, &data);

    ASSERT_THAT(err, Eq(nullptr));
}

}
