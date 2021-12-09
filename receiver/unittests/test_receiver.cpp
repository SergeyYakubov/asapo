#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockLogger.h"
#include "../src/receiver.h"

using namespace testing;
using namespace asapo;


namespace {

TEST(Receiver, Constructor) {
    asapo::Receiver receiver(nullptr, nullptr);
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(receiver.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::IO*>(receiver.io__.get()), Ne(nullptr));
}


class StartListenerFixture : public testing::Test {
  public:
    const asapo::SocketDescriptor expected_socket_descriptor = 20;
    const asapo::SocketDescriptor expected_socket_descriptor_client = 23;
    const std::string expected_address = "somehost:13579";
    const std::string expected_host = "somehost";
    const uint64_t expected_file_id = 314322;
    const uint64_t expected_file_size = 784387;
    const FileDescriptor expected_fd = 12643;

    Error err;
    ::testing::NiceMock<asapo::MockLogger> mock_logger;
    ::testing::NiceMock<asapo::MockIO> mock_io;
    asapo::Receiver receiver{nullptr, nullptr};

    void SetUp() override {
        err = nullptr;
        receiver.io__ = std::unique_ptr<asapo::IO> {&mock_io};
        receiver.log__ = &mock_logger;
    }
    void TearDown() override {
        receiver.io__.release();
    }


};

TEST_F(StartListenerFixture, CreateAndBindIPTCPSocketListenerError) {
    EXPECT_CALL(mock_io, CreateAndBindIPTCPSocketListener_t(expected_address, receiver.kMaxUnacceptedConnectionsBacklog, _))
    .WillOnce(DoAll(
                  SetArgPointee<2>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(0)
              ));

    EXPECT_CALL(mock_logger, Error(HasSubstr("prepare listener")));


    receiver.Listen(expected_address, &err, true);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}


TEST_F(StartListenerFixture, InetAcceptConnectionError) {
    EXPECT_CALL(mock_io, InetAcceptConnection_t(_, _))
    .WillOnce(DoAll(
                  SetArgPointee<1>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(new std::tuple<std::string, SocketDescriptor>(expected_address, expected_socket_descriptor_client))
              ));

    EXPECT_CALL(mock_logger, Error(HasSubstr("incoming connection")));

    receiver.Listen(expected_address, &err, true);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}

TEST_F(StartListenerFixture, Ok) {

    EXPECT_CALL(mock_io, InetAcceptConnection_t(_, _))
    .WillOnce(DoAll(
                  SetArgPointee<1>(nullptr),
                  Return(new std::tuple<std::string, SocketDescriptor>(expected_address, expected_socket_descriptor_client))
              ));

    EXPECT_CALL(mock_io, NewThread_t(_)).
    WillOnce(
        Return(nullptr)
    );

    EXPECT_CALL(mock_logger, Info(AllOf(HasSubstr("new connection"), HasSubstr(expected_host))));


    receiver.Listen(expected_address, &err, true);

    ASSERT_THAT(err, Eq(nullptr));
}


}
