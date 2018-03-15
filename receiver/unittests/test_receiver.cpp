#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/receiver.h"

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

namespace {

class StartListenerMock : public hidra2::Receiver {
  public:

};

class StartListenerFixture : public testing::Test {
  public:
    const hidra2::SocketDescriptor expected_socket_descriptor = 20;
    const std::string expected_address = "somehost:13579";
    const uint64_t expected_file_id = 314322;
    const uint64_t expected_file_size = 784387;
    const FileDescriptor expected_fd = 12643;

    Error err;

    hidra2::MockIO mockIO;
    StartListenerMock receiver;

    void SetUp() override {
        receiver.SetIO__(&mockIO);
    }
};


TEST_F(StartListenerFixture, CreateAndBindIPTCPSocketListenerError) {
    err = nullptr;

    EXPECT_CALL(mockIO, CreateAndBindIPTCPSocketListener_t(expected_address, receiver.kMaxUnacceptedConnectionsBacklog, _))
    .WillOnce(DoAll(
                  SetArgPointee<2>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(0)
              ));

    receiver.StartListener(expected_address, &err);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}


TEST_F(StartListenerFixture, Ok) {
    err = nullptr;

    EXPECT_CALL(mockIO, CreateAndBindIPTCPSocketListener_t(expected_address, receiver.kMaxUnacceptedConnectionsBacklog, _))
    .WillOnce(DoAll(
                  SetArgPointee<2>(nullptr),
                  Return(0)
              ));

    EXPECT_CALL(mockIO, NewThread_t(_))
    .WillOnce(
        Return(nullptr)
    );

    receiver.StartListener(expected_address, &err);

    ASSERT_THAT(err, Eq(nullptr));
}

}
