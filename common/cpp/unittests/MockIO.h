#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifndef HIDRA2_COMMON__MOCKIO_H
#define HIDRA2_COMMON__MOCKIO_H

namespace hidra2 {

class MockIO : public IO {
 public:
    MOCK_METHOD1(new_thread,
                 std::thread* (std::function<void()>
                     function));
    MOCK_METHOD2(GetDataFromFile,
                 FileData(
                     const std::string &fname, IOError
                     *err));
    MOCK_METHOD2(FilesInFolder,
                 std::vector<FileInfo>(
                     const std::string &folder, IOError
                     *err));
    MOCK_METHOD4(create_socket,
                 FileDescriptor(AddressFamilies
                     address_family, SocketTypes
                     socket_type, SocketProtocols
                     socket_protocol, IOError * err));
    MOCK_METHOD3(listen,
                 void(FileDescriptor
                     socket_fd, int
                     backlog, IOError * err));
    MOCK_METHOD4(inet_bind,
                 void(FileDescriptor
                     socket_fd,
                     const std::string &address, uint16_t
                     port, IOError * err));
    virtual std::unique_ptr<std::tuple<std::string, FileDescriptor>> inet_accept(FileDescriptor socket_fd, IOError * err) {
        return std::unique_ptr<std::tuple<std::string, FileDescriptor>>(inet_accept_proxy(socket_fd, err));
    };
    MOCK_METHOD2(inet_accept_proxy,
                 std::tuple<std::string, FileDescriptor>* (FileDescriptor
                     socket_fd, IOError * err));

    MOCK_METHOD3(inet_connect,
                 void(FileDescriptor
                     socket_fd,
                     const std::string &address, IOError
                     *err));
    MOCK_METHOD2(create_and_connect_ip_tcp_socket,
                 FileDescriptor(
                     const std::string &address, IOError
                     *err));
    MOCK_METHOD4(receive,
                 size_t(FileDescriptor
                     socket_fd, void * buf, size_t
                     length, IOError * err));
    MOCK_METHOD5(receive_timeout,
                 size_t(FileDescriptor
                     socket_fd, void * buf, size_t
                     length, uint16_t
                     timeout_in_sec, IOError * err));
    MOCK_METHOD4(send,
                 size_t(FileDescriptor
                     socket_fd,
                     const void* buf, size_t
                     length, IOError * err));
    MOCK_METHOD3(open,
                 FileDescriptor(
                     const std::string &filename, FileOpenMode
                     open_flags, IOError * err));
    MOCK_METHOD2(close,
                 void(FileDescriptor, IOError * err));
    MOCK_METHOD3(deprecated_read,
                 ssize_t(int
                     __fd, void * buf, size_t
                     count));
    MOCK_METHOD3(deprecated_write,
                 ssize_t(int
                     __fd,
                     const void* __buf, size_t
                     __n));

};

}

#endif //HIDRA2_COMMON__MOCKIO_H
