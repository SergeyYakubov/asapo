#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifndef HIDRA2_COMMON__MOCKIO_H
#define HIDRA2_COMMON__MOCKIO_H

namespace hidra2 {

class MockIO : public IO {
  public:
    MOCK_CONST_METHOD3(GetDataFromFile, FileData(const std::string& fname, uint64_t fsize, IOErrors* err));
    MOCK_CONST_METHOD2(FilesInFolder, std::vector<FileInfo>(const std::string& folder, IOErrors* err));
    MOCK_CONST_METHOD1(NewThread, std::thread * (std::function<void()> function));
    MOCK_CONST_METHOD4(CreateSocket, FileDescriptor(AddressFamilies address_family, SocketTypes socket_type,
                                                    SocketProtocols socket_protocol, IOErrors* err));
    MOCK_CONST_METHOD3(Listen, void(FileDescriptor socket_fd, int backlog, IOErrors* err));
    MOCK_CONST_METHOD4(InetBind, void(FileDescriptor socket_fd, const std::string& address, uint16_t port, IOErrors* err));
    virtual std::unique_ptr<std::tuple<std::string, FileDescriptor>> InetAccept(FileDescriptor socket_fd,
    IOErrors* err) const {
        return std::unique_ptr<std::tuple<std::string, FileDescriptor>>(InetAccept_proxy(socket_fd, err));
    }
    MOCK_CONST_METHOD2(InetAccept_proxy, std::tuple<std::string, FileDescriptor>* (FileDescriptor socket_fd,
                       IOErrors* err));
    MOCK_CONST_METHOD3(InetConnect, void(FileDescriptor socket_fd, const std::string& address, IOErrors* err));
    MOCK_CONST_METHOD2(CreateAndConnectIPTCPSocket, FileDescriptor(const std::string& address, IOErrors* err));
    MOCK_CONST_METHOD4(Receive, size_t(FileDescriptor socket_fd, void* buf, size_t length, IOErrors* err));
    MOCK_CONST_METHOD5(ReceiveTimeout, size_t(FileDescriptor socket_fd, void* buf, size_t length, uint16_t timeout_in_sec,
                                              IOErrors* err));
    MOCK_CONST_METHOD4(Send, size_t(FileDescriptor socket_fd, const void* buf, size_t length, IOErrors* err));
    MOCK_CONST_METHOD3(Skip, void(FileDescriptor socket_fd, size_t length, IOErrors* err));
    MOCK_CONST_METHOD3(Open, FileDescriptor(const std::string& filename, int open_flags, IOErrors* err));
    MOCK_CONST_METHOD2(Close, void(FileDescriptor, IOErrors*));
    MOCK_CONST_METHOD4(Write, size_t(FileDescriptor fd, const void* buf, size_t length, IOErrors* err));
    MOCK_CONST_METHOD2(CreateDirectory, void(const std::string& directory_name, hidra2::IOErrors* err));
};

}

#endif //HIDRA2_COMMON__MOCKIO_H
