#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <netinet/in.h>
#include "../src/producer_impl.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
//using ::testing::SetArg;
using ::testing::Gt;

class MockIO : public hidra2::IO {
 public:
    MOCK_METHOD4(create_socket,
                 hidra2::FileDescriptor(hidra2::AddressFamilies address_family, hidra2::SocketTypes socket_type, hidra2::SocketProtocols socket_protocol, hidra2::IOErrors* err));
    MOCK_METHOD3(listen,
                 void(hidra2::FileDescriptor socket_fd, int backlog, hidra2::IOErrors* err));
    MOCK_METHOD4(inet_bind,
                 void(hidra2::FileDescriptor socket_fd, const std::string& address, uint16_t port, hidra2::IOErrors* err));
    MOCK_METHOD2(inet_accept,
                 std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>>(hidra2::FileDescriptor socket_fd, hidra2::IOErrors* err));
    MOCK_METHOD3(inet_connect,
                 void(hidra2::FileDescriptor socket_fd, const std::string& address, hidra2::IOErrors* err));
    MOCK_METHOD2(create_and_connect_ip_tcp_socket,
                 hidra2::FileDescriptor(const std::string& address, hidra2::IOErrors* err));
    MOCK_METHOD4(receive,
                 size_t(hidra2::FileDescriptor socket_fd, void* buf, size_t length, hidra2::IOErrors* err));
    MOCK_METHOD5(receive_timeout,
                 size_t(hidra2::FileDescriptor socket_fd, void* buf, size_t length, uint16_t timeout_in_sec, hidra2::IOErrors* err));
    MOCK_METHOD4(send,
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
                 hidra2::FileData(const std::string &fname, hidra2::IOErrors* err));
    MOCK_METHOD2(FilesInFolder,
                 std::vector<hidra2::FileInfo>(const std::string& folder, hidra2::IOErrors* err));
};

TEST(get_version, VersionAboveZero) {
    hidra2::ProducerImpl producer;
    EXPECT_GE(producer.get_version(), 0);
}


ACTION(write_hello_reponse_arg1) {
    ((hidra2::HelloResponse*)arg1)->error_code = hidra2::NET_ERR__NO_ERROR;
}

TEST(ProducerImpl, connect_to_receiver) {
    MockIO mockIO;

    hidra2::ProducerImpl producer;
    producer.__set_io(&mockIO);

    std::string expected_address = "127.0.0.1:9090";
    int         expected_fd = 83942;

    EXPECT_CALL(mockIO, create_and_connect_ip_tcp_socket(expected_address, _))
        .WillOnce(DoAll(testing::SetArgPointee<1>(hidra2::IOErrors::NO_ERROR), Return(expected_fd)));

    EXPECT_CALL(mockIO, send(expected_fd, _/*addr*/, sizeof(hidra2::HelloRequest), _))
        .WillOnce(DoAll(testing::SetArgPointee<3>(hidra2::IOErrors::NO_ERROR), Return(sizeof(hidra2::HelloRequest))));



    EXPECT_CALL(mockIO, receive_timeout(expected_fd, _/*addr*/, sizeof(hidra2::HelloResponse), Gt(0), _))
        .WillOnce(DoAll(write_hello_reponse_arg1(), testing::SetArgPointee<4>(hidra2::IOErrors::NO_ERROR), Return(sizeof(hidra2::HelloResponse))));

    EXPECT_EQ(producer.connect_to_receiver(expected_address), hidra2::PRODUCER_ERROR__OK);
}
}
