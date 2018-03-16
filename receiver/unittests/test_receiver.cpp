#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/receiver.h"
#include "../src/receiver_error.h"
#include "../src/network_producer_peer_impl.h"

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
using ::hidra2::NetworkProducerPeer;
using ::hidra2::SocketDescriptor;

namespace {

class StartListenerMock : public hidra2::Receiver {
  public:

};

class StartListenerFixture : public testing::Test {
  public:
    const hidra2::SocketDescriptor expected_socket_descriptor = 20;
    const hidra2::SocketDescriptor expected_socket_descriptor_client = 23;
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


TEST_F(StartListenerFixture, AlreadyListening) {
    err = nullptr;

    EXPECT_CALL(mockIO, CreateAndBindIPTCPSocketListener_t(expected_address, receiver.kMaxUnacceptedConnectionsBacklog, _))
        .Times(1)
        .WillOnce(DoAll(
            SetArgPointee<2>(nullptr),
            Return(0)
        ));

    EXPECT_CALL(mockIO, NewThread_t(_))
        .Times(1)
        .WillOnce(
            Return(nullptr)
        );

    receiver.StartListener(expected_address, &err);

    ASSERT_THAT(err, Eq(nullptr));

    receiver.StartListener(expected_address, &err);

    ASSERT_THAT(err, Eq(hidra2::ReceiverErrorTemplates::kAlreadyListening));
}


class AcceptThreadLogicWorkMock : public StartListenerMock {
  public:

    std::unique_ptr<NetworkProducerPeer> CreateNewPeer(int peer_socket_fd,
            const std::string& address) const noexcept override {
        std::unique_ptr<NetworkProducerPeer> networkProducerPeer;
        auto data = CreateNewPeer_t(peer_socket_fd, address);
        networkProducerPeer.reset(data);
        return networkProducerPeer;
    }
    MOCK_CONST_METHOD2(CreateNewPeer_t, NetworkProducerPeer * (int peer_socket_fd, const std::string& address));
};

class AcceptThreadLogicWorkFixture : public StartListenerFixture {
  public:
    AcceptThreadLogicWorkMock receiver;

    void SetUp() override {
        receiver.SetIO__(&mockIO);
    }
};

TEST_F(AcceptThreadLogicWorkFixture, InetAcceptConnectionError) {
    err = nullptr;

    EXPECT_CALL(mockIO, InetAcceptConnection_t(-1, _))
    .WillOnce(DoAll(
                  SetArgPointee<1>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(new std::tuple<std::string, SocketDescriptor>(expected_address, expected_socket_descriptor_client))
              ));

    receiver.AcceptThreadLogicWork(&err);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST_F(AcceptThreadLogicWorkFixture, OkAndCheckListSize) {
    err = nullptr;

    EXPECT_CALL(mockIO, InetAcceptConnection_t(-1, _))
    .WillOnce(DoAll(
                  SetArgPointee<1>(nullptr),
                  Return(new std::tuple<std::string, SocketDescriptor>(expected_address, expected_socket_descriptor_client))
              ));

    EXPECT_CALL(receiver, CreateNewPeer_t(expected_socket_descriptor_client, expected_address))
    .WillOnce(
        Return(new hidra2::NetworkProducerPeerImpl(expected_socket_descriptor_client, expected_address))
    );

    receiver.AcceptThreadLogicWork(&err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(receiver.GetConnectedPeers().size(), 1);
}

class StopListenerMock : public StartListenerMock {
 public:

};

class StopListenerFixture : public StartListenerFixture {
 public:
    StopListenerMock receiver;

    void SetUp() override {
        receiver.SetIO__(&mockIO);
    }
};

TEST_F(StopListenerFixture, InetAcceptConnectionError) {
    err = nullptr;

    EXPECT_CALL(mockIO, CloseSocket_t(-1, _))
        .WillOnce(
            SetArgPointee<1>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release())
        );

    receiver.StopListener(&err);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}



}
