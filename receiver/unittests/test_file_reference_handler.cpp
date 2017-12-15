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
    MOCK_METHOD4(create_socket,
                 hidra2::FileDescriptor(hidra2::AddressFamilies
                     address_family, hidra2::SocketTypes
                     socket_type,
                         hidra2::SocketProtocols
                     socket_protocol, hidra2::IOErrors * err));
    MOCK_METHOD3(listen,
                 void(hidra2::FileDescriptor
                     socket_fd, int
                     backlog, hidra2::IOErrors * err));
    MOCK_METHOD4(inet_bind,
                 void(hidra2::FileDescriptor
                     socket_fd,
                     const std::string &address, uint16_t
                     port, hidra2::IOErrors * err));

    virtual std::unique_ptr<std::tuple<std::string,
                                       hidra2::FileDescriptor>> inet_accept(hidra2::FileDescriptor socket_fd,
                                                                            hidra2::IOError* err) {
        return std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>>(inet_accept_proxy(socket_fd, err));
    };

    MOCK_METHOD2(inet_accept_proxy,
                 std::tuple<std::string, hidra2::FileDescriptor>* (hidra2::FileDescriptor
                     socket_fd,
                         hidra2::IOErrors * err));
    MOCK_METHOD3(inet_connect,
                 void(hidra2::FileDescriptor
                     socket_fd,
                     const std::string &address, hidra2::IOErrors
                     *err));
    MOCK_METHOD2(create_and_connect_ip_tcp_socket,
                 hidra2::FileDescriptor(
                     const std::string &address, hidra2::IOErrors
                     *err));
    MOCK_METHOD4(receive,
                 size_t(hidra2::FileDescriptor
                     socket_fd, void * buf, size_t
                     length, hidra2::IOErrors * err));
    MOCK_METHOD5(receive_timeout,
                 size_t(hidra2::FileDescriptor
                     socket_fd, void * buf, size_t
                     length, uint16_t
                     timeout_in_sec, hidra2::IOErrors * err));
    MOCK_METHOD4(send,
                 size_t(hidra2::FileDescriptor
                     socket_fd,
                     const void* buf, size_t
                     length, hidra2::IOErrors * err));
    MOCK_METHOD2(deprecated_open,
                 int(
                     const char* __file,
                     int __oflag));
    MOCK_METHOD1(deprecated_close,
                 int(int
                     __fd));
    MOCK_METHOD3(deprecated_read,
                 ssize_t(int
                     __fd, void * buf, size_t
                     count));
    MOCK_METHOD3(deprecated_write,
                 ssize_t(int
                     __fd,
                     const void* __buf, size_t
                     __n));
    MOCK_METHOD2(GetDataFromFile,
                 hidra2::FileData(
                     const std::string &fname, hidra2::IOErrors
                     *err));
    MOCK_METHOD2(FilesInFolder,
                 std::vector<hidra2::FileInfo>(
                     const std::string &folder, hidra2::IOErrors
                     *err));
};


TEST(FileReferenceHandler, add_file)

}
