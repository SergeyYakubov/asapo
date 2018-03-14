#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/receiver.h"

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Mock;
using ::testing::InSequence;

namespace {

TEST(a, b) {
    hidra2::MockIO mockIO;

    hidra2::Receiver receiver;

}

/*
    TEST(Receiver, start_Listener__InetBind_fail) {
    hidra2::MockIO mockIO;

    hidra2::Receiver receiver;

    receiver.SetIO__(&mockIO);

    InSequence sequence;

    std::string expected_address = "127.0.0.1:9876";

    EXPECT_CALL(mockIO, CreateSocket(hidra2::AddressFamilies::INET, hidra2::SocketTypes::STREAM,
                 hidra2::SocketProtocols::IP, _))
    .Times(1)
    .WillOnce(
    DoAll(
    testing::SetArgPointee<3>(hidra2::IOErrors::kUnknownIOError),
    Return(-1)
    ));

    hidra2::ReceiverError receiver_error;
    receiver.StartListener(expected_address, &receiver_error);
    EXPECT_EQ(receiver_error, hidra2::ReceiverError::FailToCreateSocket);
}
*/

}
