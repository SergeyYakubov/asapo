#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/receiver.h"
#include "../src/receiver_error.h"
#include "../src/connection.h"

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::SetArgPointee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Mock;
using ::testing::InSequence;
using ::hidra2::Error;
using ::hidra2::FileDescriptor;
using ::hidra2::ErrorInterface;
using ::hidra2::Connection;
using ::hidra2::SocketDescriptor;

namespace {

class StartListenerFixture : public testing::Test {
  public:
    const hidra2::SocketDescriptor expected_socket_descriptor = 20;
    const hidra2::SocketDescriptor expected_socket_descriptor_client = 23;
    const std::string expected_address = "somehost:13579";
    const uint64_t expected_file_id = 314322;
    const uint64_t expected_file_size = 784387;
    const FileDescriptor expected_fd = 12643;

    Error err;

    ::testing::NiceMock<hidra2::MockIO> mockIO;
    hidra2::Receiver receiver;

    void SetUp() override {
        err = nullptr;
        receiver.SetIO__(&mockIO);
    }
};

TEST_F(StartListenerFixture, CreateAndBindIPTCPSocketListenerError) {
    EXPECT_CALL(mockIO, CreateAndBindIPTCPSocketListener_t(expected_address, receiver.kMaxUnacceptedConnectionsBacklog, _))
    .WillOnce(DoAll(
                  SetArgPointee<2>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(0)
              ));

    receiver.Listen(expected_address, &err, true);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}


TEST_F(StartListenerFixture, InetAcceptConnectionError) {
    EXPECT_CALL(mockIO, InetAcceptConnection_t(_, _))
    .WillOnce(DoAll(
                  SetArgPointee<1>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(new std::tuple<std::string, SocketDescriptor>(expected_address, expected_socket_descriptor_client))
              ));

    receiver.Listen(expected_address, &err, true);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST_F(StartListenerFixture, Ok) {

    EXPECT_CALL(mockIO, InetAcceptConnection_t(_, _))
    .WillOnce(DoAll(
                  SetArgPointee<1>(nullptr),
                  Return(new std::tuple<std::string, SocketDescriptor>(expected_address, expected_socket_descriptor_client))
              ));

    EXPECT_CALL(mockIO, NewThread_t(_)).
    WillOnce(
        Return(nullptr)
    );

    receiver.Listen(expected_address, &err, true);

    ASSERT_THAT(err, Eq(nullptr));
}


}
