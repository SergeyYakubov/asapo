#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <system_wrappers/io.h>
#include "../src/receiver.h"
#include "../../common/cpp/unittests/MockIO.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Mock;
using ::testing::InSequence;

//Need to be redone completely

// hidra2::Receiver receiver;
// hidra2::FileDescriptor  expected_socket_fd  = 1338;
//
//TEST(Receiver, start_Listener__CreateSocket_fail) {
//    hidra2::MockIO mockIO;
//    receiver.__set_io(&mockIO);
//
//    InSequence sequence;
//
//    std::string expected_address    = "127.0.0.1";
//    uint16_t expected_port          = 9876;
//
//    EXPECT_CALL(mockIO, CreateSocket(hidra2::AddressFamilies::INET, hidra2::SocketTypes::STREAM,
//                                      hidra2::SocketProtocols::IP, _))
//    .Times(1)
//    .WillOnce(
//        DoAll(
//            testing::SetArgPointee<3>(hidra2::IOError::UNKNOWN_ERROR),
//            Return(-1)
//        ));
//
//    hidra2::ReceiverError receiver_error;
//    receiver.StartListener(expected_address, expected_port, &receiver_error);
//    EXPECT_EQ(receiver_error, hidra2::ReceiverError::FAILED_CREATING_SOCKET);
//
//    Mock::VerifyAndClearExpectations(&mockIO);
//}
//
//TEST(Receiver, start_Listener__InetBind_fail) {
//    hidra2::MockIO mockIO;
//    receiver.__set_io(&mockIO);
//
//    InSequence sequence;
//
//    std::string expected_address    = "127.0.0.1";
//    uint16_t    expected_port       = 9876;
//
//    EXPECT_CALL(mockIO, CreateSocket(hidra2::AddressFamilies::INET, hidra2::SocketTypes::STREAM,
//                                      hidra2::SocketProtocols::IP, _))
//    .Times(1)
//    .WillOnce(
//        DoAll(
//            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
//            Return(expected_socket_fd)
//        ));
//
//    EXPECT_CALL(mockIO, InetBind(expected_socket_fd, expected_address, expected_port, _))
//    .Times(1)
//    .WillOnce(testing::SetArgPointee<3>(hidra2::IOError::ADDRESS_ALREADY_IN_USE));
//
//    EXPECT_CALL(mockIO, Close(expected_socket_fd))
//    .Times(1);
//
//    hidra2::ReceiverError receiver_error;
//    receiver.StartListener(expected_address, expected_port, &receiver_error);
//    EXPECT_EQ(receiver_error, hidra2::ReceiverError::FAILED_CREATING_SOCKET);
//
//    Mock::VerifyAndClearExpectations(&mockIO);
//}
//
//TEST(Receiver, start_Listener__Listen_fail) {
//    hidra2::MockIO mockIO;
//    receiver.__set_io(&mockIO);
//
//    InSequence sequence;
//
//    std::string expected_address    = "127.0.0.1";
//    uint16_t    expected_port       = 9876;
//
//    EXPECT_CALL(mockIO, CreateSocket(hidra2::AddressFamilies::INET, hidra2::SocketTypes::STREAM,
//                                      hidra2::SocketProtocols::IP, _))
//    .Times(1)
//    .WillOnce(
//        DoAll(
//            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
//            Return(expected_socket_fd)
//        ));
//
//    EXPECT_CALL(mockIO, InetBind(expected_socket_fd, expected_address, expected_port, _))
//    .Times(1)
//    .WillOnce(testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR));
//
//    EXPECT_CALL(mockIO, Listen(expected_socket_fd, receiver.kMaxUnacceptedConnectionsBacklog, _))
//    .Times(1)
//    .WillOnce(testing::SetArgPointee<2>(hidra2::IOError::BAD_FILE_NUMBER));
//
//    EXPECT_CALL(mockIO, Close(expected_socket_fd, _))
//    .Times(1);
//
//    hidra2::ReceiverError receiver_error;
//    receiver.StartListener(expected_address, expected_port, &receiver_error);
//    EXPECT_EQ(receiver_error, hidra2::ReceiverError::FAILED_CREATING_SOCKET);
//
//    Mock::VerifyAndClearExpectations(&mockIO);
//}
//
//TEST(Receiver, start_Listener) {
//    hidra2::MockIO mockIO;
//    receiver.__set_io(&mockIO);
//
//    InSequence sequence;
//
//    std::string expected_address    = "127.0.0.1";
//    uint16_t    expected_port       = 9876;
//
//    EXPECT_CALL(mockIO, CreateSocket(hidra2::AddressFamilies::INET, hidra2::SocketTypes::STREAM,
//                                      hidra2::SocketProtocols::IP, _))
//    .Times(1)
//    .WillOnce(
//        DoAll(
//            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
//            Return(expected_socket_fd)
//        ));
//
//    EXPECT_CALL(mockIO, InetBind(expected_socket_fd, expected_address, expected_port, _))
//    .Times(1)
//    .WillOnce(testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR));
//
//    EXPECT_CALL(mockIO, Listen(expected_socket_fd, receiver.kMaxUnacceptedConnectionsBacklog, _))
//    .Times(1)
//    .WillOnce(testing::SetArgPointee<2>(hidra2::IOError::NO_ERROR));
//
//    /**
//     * TODO: Since StartListener will start a new thread
//     * we need to mock std::thread
//     */
//    EXPECT_CALL(mockIO, InetAcceptProxy(expected_socket_fd, _))
//    .Times(1)
//    .WillOnce(
//        DoAll(
//            testing::SetArgPointee<1>(hidra2::IOError::BAD_FILE_NUMBER),
//            Return(nullptr)
//        ));
//
//
//    hidra2::ReceiverError receiver_error;
//    receiver.StartListener(expected_address, expected_port, &receiver_error);
//    EXPECT_EQ(receiver_error, hidra2::ReceiverError::NO_ERROR);
//
//    sleep(1); //Make sure that the thread is running so InetAcceptProxy would work
//
//    Mock::VerifyAndClearExpectations(&mockIO);
//}
//
//TEST(Receiver, start_Listener_already_Listening) {
//    InSequence sequence;
//
//    std::string expected_address    = "127.0.0.1";
//    uint16_t    expected_port       = 9876;
//
//    hidra2::ReceiverError receiver_error;
//    receiver.StartListener(expected_address, expected_port, &receiver_error);
//    EXPECT_EQ(receiver_error, hidra2::ReceiverError::ALREADY_LISTEING);
//}
//
//TEST(Receiver, stop_Listener) {
//    hidra2::MockIO mockIO;
//    receiver.__set_io(&mockIO);
//
//
//    EXPECT_CALL(mockIO, Close(expected_socket_fd))
//    .Times(1)
//    .WillOnce(
//        Return(0)
//    );
//
//
//    hidra2::ReceiverError receiver_error;
//
//    receiver.stop_listener(&receiver_error);
//    EXPECT_EQ(receiver_error, hidra2::ReceiverError::NO_ERROR);
//
//    Mock::VerifyAndClearExpectations(&mockIO);
//}

}
