#include <gmock/gmock.h>
#include "gtest/gtest.h"

#include "asapo/io/io.h"
#include "asapo/unittests/MockIO.h"
#include "mocking.h"
#include "../src/tcp_connection_pool.h"
#include "../../../../common/cpp/src/system_io/system_io.h"


using asapo::IO;
using asapo::MessageMeta;
using asapo::MessageData;
using asapo::MockIO;
using asapo::SimpleError;
using asapo::TcpConnectionPool;
using asapo::SocketDescriptor;
using asapo::Error;

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

TEST(TcpConnectioPool, Constructor) {
    auto client = std::unique_ptr<TcpConnectionPool> {new TcpConnectionPool()};
    ASSERT_THAT(dynamic_cast<asapo::SystemIO*>(client->io__.get()), Ne(nullptr));
}


class TcpConnectioPoolTests : public Test {
  public:
    NiceMock<MockIO> mock_io;
    MessageMeta info;
    std::string expected_source = "test:8400";
    TcpConnectionPool pool;
    SocketDescriptor expected_sd = 123;
    bool reused;
    void SetUp() override {
        pool.io__ =  std::unique_ptr<IO> {&mock_io};
    }
    void TearDown() override {
        pool.io__.release();
    }

    void ExpectSingleConnect() {
        EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_source, _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(nullptr),
                Return(expected_sd)
            ));
    }
    void ExpectTwoConnects(bool second_fails = false) {
        EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_source, _)).Times(2)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(nullptr),
                Return(expected_sd)
            ))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(second_fails ? asapo::IOErrorTemplates::kUnknownIOError.Generate().release() : nullptr),
                Return(second_fails ? asapo::kDisconnectedSocketDescriptor : expected_sd + 1)
            ));
    }

};

TEST_F(TcpConnectioPoolTests, GetConnectionCreatesNewOne) {
    ExpectSingleConnect();

    Error err;
    auto sd = pool.GetFreeConnection(expected_source, &reused, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(reused, Eq(false));
    ASSERT_THAT(sd, Eq(expected_sd));
}


TEST_F(TcpConnectioPoolTests, GetTwoConnection) {
    ExpectTwoConnects();

    Error err;
    auto sd1 = pool.GetFreeConnection(expected_source, &reused, &err);
    auto sd2 = pool.GetFreeConnection(expected_source, &reused, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(sd1, Eq(expected_sd));
    ASSERT_THAT(sd2, Eq(expected_sd + 1));
}


TEST_F(TcpConnectioPoolTests, GetConnectionUsesConnectionPool) {
    ExpectSingleConnect();

    Error err;
    auto sd1 = pool.GetFreeConnection(expected_source, &reused, &err);
    pool.ReleaseConnection(sd1);
    auto sd2 = pool.GetFreeConnection(expected_source, &reused, &err);

    ASSERT_THAT(reused, Eq(true));
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(sd1, Eq(expected_sd));
    ASSERT_THAT(sd2, Eq(expected_sd));
}

TEST_F(TcpConnectioPoolTests, CannotConnect) {

    EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_source, _))
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(asapo::IOErrorTemplates::kInvalidAddressFormat.Generate().release()),
            Return(asapo::kDisconnectedSocketDescriptor)
        ));

    Error err, err1;
    auto sd = pool.GetFreeConnection(expected_source, &reused, &err);
    auto sd1 = pool.Reconnect(sd, &err1);
    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kInvalidAddressFormat));
    ASSERT_THAT(err1, Ne(nullptr));
    ASSERT_THAT(sd, Eq(asapo::kDisconnectedSocketDescriptor));
    ASSERT_THAT(sd1, Eq(asapo::kDisconnectedSocketDescriptor));
}

TEST_F(TcpConnectioPoolTests, CanReconnect) {
    ExpectTwoConnects();

    Error err;
    auto sd1 = pool.GetFreeConnection(expected_source, &reused, &err);
    pool.ReleaseConnection(sd1);
    auto sd2 = pool.Reconnect(sd1, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(sd1, Eq(expected_sd));
    ASSERT_THAT(sd2, Eq(expected_sd + 1));
}

TEST_F(TcpConnectioPoolTests, ReconnectionFails) {
    ExpectTwoConnects(true);

    Error err1, err2, err3;
    auto sd1 = pool.GetFreeConnection(expected_source, &reused, &err1);
    pool.ReleaseConnection(sd1);
    auto sd2 = pool.Reconnect(sd1, &err2);
    auto sd3 = pool.Reconnect(sd2, &err3); // this reconnect should not work as the record has been removed

    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(sd1, Eq(expected_sd));
    ASSERT_THAT(err2, Eq(asapo::IOErrorTemplates::kUnknownIOError));
    ASSERT_THAT(sd2, Eq(asapo::kDisconnectedSocketDescriptor));
    ASSERT_THAT(sd3, Eq(asapo::kDisconnectedSocketDescriptor));
    ASSERT_THAT(err3, Ne(nullptr));
}


TEST_F(TcpConnectioPoolTests, ReconnectWrongSD) {
    Error err;
    pool.Reconnect(expected_sd, &err);

    ASSERT_THAT(err, Ne(nullptr));
}


}
