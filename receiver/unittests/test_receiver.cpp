#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <system_wrappers/io.h>
#include "../src/receiver.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Mock;
using ::testing::InSequence;

class MockIO : public hidra2::IO {
  public:
    MOCK_METHOD4(CreateSocket,
                 hidra2::FileDescriptor(hidra2::AddressFamilies address_family, hidra2::SocketTypes socket_type,
                                        hidra2::SocketProtocols socket_protocol, hidra2::IOErrors* err));
    MOCK_METHOD3(Listen,
                 void(hidra2::FileDescriptor socket_fd, int backlog, hidra2::IOErrors* err));
    MOCK_METHOD4(InetBind,
                 void(hidra2::FileDescriptor socket_fd, const std::string& address, uint16_t port, hidra2::IOErrors* err));

    virtual std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>> InetAccept(hidra2::FileDescriptor socket_fd,
    hidra2::IOError* err) {
        return std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>>(inet_accept_proxy(socket_fd, err));
    };

    MOCK_METHOD2(inet_accept_proxy,
                 std::tuple<std::string, hidra2::FileDescriptor>* (hidra2::FileDescriptor socket_fd,
                         hidra2::IOErrors* err));
    MOCK_METHOD3(InetConnect,
                 void(hidra2::FileDescriptor socket_fd, const std::string& address, hidra2::IOErrors* err));
    MOCK_METHOD2(CreateAndConnectIPTCPSocket,
                 hidra2::FileDescriptor(const std::string& address, hidra2::IOErrors* err));
    MOCK_METHOD4(Receive,
                 size_t(hidra2::FileDescriptor socket_fd, void* buf, size_t length, hidra2::IOErrors* err));
    MOCK_METHOD5(ReceiveTimeout,
                 size_t(hidra2::FileDescriptor socket_fd, void* buf, size_t length, uint16_t timeout_in_sec, hidra2::IOErrors* err));
    MOCK_METHOD4(Send,
                 size_t(hidra2::FileDescriptor socket_fd, const void* buf, size_t length, hidra2::IOErrors* err));
    MOCK_METHOD2(deprecated_open,
                 int(const char* __file, int __oflag));
    MOCK_METHOD1(deprecated_close,
                 int(int __fd));
    MOCK_METHOD3(deprecated_read,
                 ssize_t(int __fd, void* buf, size_t count));
    MOCK_METHOD3(deprecated_write,
                 ssize_t(int __fd, const void* __buf, size_t __n));
    MOCK_METHOD2(GetDataFromFile,
                 hidra2::FileData(const std::string& fname, hidra2::IOErrors* err));
    MOCK_METHOD2(FilesInFolder,
                 std::vector<hidra2::FileInfo>(const std::string& folder, hidra2::IOErrors* err));
};

hidra2::Receiver receiver;
hidra2::FileDescriptor  expected_socket_fd  = 1338;

TEST(Receiver, start_listener__create_socket_fail) {
    MockIO mockIO;
    receiver.__set_io(&mockIO);

    InSequence sequence;

    std::string expected_address    = "127.0.0.1";
    uint16_t expected_port          = 9876;

    EXPECT_CALL(mockIO, create_socket(hidra2::AddressFamilies::INET, hidra2::SocketTypes::STREAM,
                                      hidra2::SocketProtocols::IP, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::UNKNOWN_ERROR),
            Return(-1)
        ));

    hidra2::ReceiverError receiver_error;
    receiver.start_listener(expected_address, expected_port, &receiver_error);
    EXPECT_EQ(receiver_error, hidra2::ReceiverError::FAILED_CREATING_SOCKET);

    Mock::VerifyAndClearExpectations(&mockIO);
}

TEST(Receiver, start_listener__inet_bind_fail) {
    MockIO mockIO;
    receiver.__set_io(&mockIO);

    InSequence sequence;

    std::string expected_address    = "127.0.0.1";
    uint16_t    expected_port       = 9876;

    EXPECT_CALL(mockIO, create_socket(hidra2::AddressFamilies::INET, hidra2::SocketTypes::STREAM,
                                      hidra2::SocketProtocols::IP, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(expected_socket_fd)
        ));

    EXPECT_CALL(mockIO, inet_bind(expected_socket_fd, expected_address, expected_port, _))
    .Times(1)
    .WillOnce(testing::SetArgPointee<3>(hidra2::IOError::ADDRESS_ALREADY_IN_USE));

    EXPECT_CALL(mockIO, deprecated_close(expected_socket_fd))
    .Times(1);

    hidra2::ReceiverError receiver_error;
    receiver.start_listener(expected_address, expected_port, &receiver_error);
    EXPECT_EQ(receiver_error, hidra2::ReceiverError::FAILED_CREATING_SOCKET);

    Mock::VerifyAndClearExpectations(&mockIO);
}

TEST(Receiver, start_listener__listen_fail) {
    MockIO mockIO;
    receiver.__set_io(&mockIO);

    InSequence sequence;

    std::string expected_address    = "127.0.0.1";
    uint16_t    expected_port       = 9876;

    EXPECT_CALL(mockIO, create_socket(hidra2::AddressFamilies::INET, hidra2::SocketTypes::STREAM,
                                      hidra2::SocketProtocols::IP, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(expected_socket_fd)
        ));

    EXPECT_CALL(mockIO, inet_bind(expected_socket_fd, expected_address, expected_port, _))
    .Times(1)
    .WillOnce(testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR));

    EXPECT_CALL(mockIO, listen(expected_socket_fd, receiver.kMaxUnacceptedConnectionsBacklog, _))
    .Times(1)
    .WillOnce(testing::SetArgPointee<2>(hidra2::IOError::BAD_FILE_NUMBER));

    EXPECT_CALL(mockIO, deprecated_close(expected_socket_fd))
    .Times(1);

    hidra2::ReceiverError receiver_error;
    receiver.start_listener(expected_address, expected_port, &receiver_error);
    EXPECT_EQ(receiver_error, hidra2::ReceiverError::FAILED_CREATING_SOCKET);

    Mock::VerifyAndClearExpectations(&mockIO);
}

TEST(Receiver, start_listener) {
    MockIO mockIO;
    receiver.__set_io(&mockIO);

    InSequence sequence;

    std::string expected_address    = "127.0.0.1";
    uint16_t    expected_port       = 9876;

    EXPECT_CALL(mockIO, create_socket(hidra2::AddressFamilies::INET, hidra2::SocketTypes::STREAM,
                                      hidra2::SocketProtocols::IP, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(expected_socket_fd)
        ));

    EXPECT_CALL(mockIO, inet_bind(expected_socket_fd, expected_address, expected_port, _))
    .Times(1)
    .WillOnce(testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR));

    EXPECT_CALL(mockIO, listen(expected_socket_fd, receiver.kMaxUnacceptedConnectionsBacklog, _))
    .Times(1)
    .WillOnce(testing::SetArgPointee<2>(hidra2::IOError::NO_ERROR));

    /**
     * TODO: Since start_listener will start a new thread
     * we need to mock std::thread
     */
    EXPECT_CALL(mockIO, inet_accept_proxy(expected_socket_fd, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(nullptr)
        ));


    hidra2::ReceiverError receiver_error;
    receiver.start_listener(expected_address, expected_port, &receiver_error);
    EXPECT_EQ(receiver_error, hidra2::ReceiverError::NO_ERROR);

    sleep(1); //Make sure that the thread is running so inet_accept_proxy would work

    Mock::VerifyAndClearExpectations(&mockIO);
}

TEST(Receiver, start_listener_already_listening) {
    InSequence sequence;

    std::string expected_address    = "127.0.0.1";
    uint16_t    expected_port       = 9876;

    hidra2::ReceiverError receiver_error;
    receiver.start_listener(expected_address, expected_port, &receiver_error);
    EXPECT_EQ(receiver_error, hidra2::ReceiverError::ALREADY_LISTEING);
}

TEST(Receiver, stop_listener) {
    MockIO mockIO;
    receiver.__set_io(&mockIO);


    EXPECT_CALL(mockIO, deprecated_close(expected_socket_fd))
    .Times(1)
    .WillOnce(
        Return(0)
    );


    hidra2::ReceiverError receiver_error;

    receiver.stop_listener(&receiver_error);
    EXPECT_EQ(receiver_error, hidra2::ReceiverError::NO_ERROR);

    Mock::VerifyAndClearExpectations(&mockIO);
}

}
