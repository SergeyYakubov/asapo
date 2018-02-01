#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifndef HIDRA2_COMMON__MOCKIO_H
#define HIDRA2_COMMON__MOCKIO_H

namespace hidra2 {
class MockIO : public IO {
  public:
    MOCK_CONST_METHOD1(NewThread,
                       std::thread * (std::function<void()> function));
    MOCK_CONST_METHOD4(CreateSocket,
                       SocketDescriptor(AddressFamilies address_family, SocketTypes socket_type, SocketProtocols socket_protocol,
                                        IOErrors* err));
    MOCK_CONST_METHOD3(Listen,
                       void(SocketDescriptor socket_fd, int backlog, IOErrors* err));
    MOCK_CONST_METHOD3(InetBind,
                       void(SocketDescriptor socket_fd, const std::string& address, IOErrors* err));
    virtual std::unique_ptr<std::tuple<std::string, SocketDescriptor>> InetAccept(SocketDescriptor socket_fd,
    IOErrors* err) const {
        return std::unique_ptr<std::tuple<std::string, SocketDescriptor>>(InetAccept_proxy(socket_fd, err));
    }
    MOCK_CONST_METHOD2(InetAccept_proxy, std::tuple<std::string, SocketDescriptor>* (SocketDescriptor socket_fd,
                       IOErrors* err));
    MOCK_CONST_METHOD2(ResolveHostnameToIp, std::string(const std::string& hostname, IOErrors* err));
    MOCK_CONST_METHOD3(InetConnect,
                       void(SocketDescriptor socket_fd, const std::string& address, IOErrors* err));
    MOCK_CONST_METHOD2(CreateAndConnectIPTCPSocket,
                       SocketDescriptor(const std::string& address, IOErrors* err));
    MOCK_CONST_METHOD4(Receive,
                       size_t(SocketDescriptor socket_fd, void* buf, size_t length, IOErrors* err));
    MOCK_CONST_METHOD5(ReceiveTimeout,
                       size_t(SocketDescriptor socket_fd, void* buf, size_t length, long timeout_in_usec, IOErrors* err));
    MOCK_CONST_METHOD4(Send,
                       size_t(SocketDescriptor socket_fd, const void* buf, size_t length, IOErrors* err));
    MOCK_CONST_METHOD3(Skip,
                       void(SocketDescriptor socket_fd, size_t length, IOErrors* err));
    MOCK_CONST_METHOD2(CloseSocket,
                       void(SocketDescriptor socket_fd, IOErrors* err));
    MOCK_CONST_METHOD3(Open,
                       FileDescriptor(const std::string& filename, int open_flags, IOErrors* err));
    MOCK_CONST_METHOD2(Close,
                       void(FileDescriptor fd, IOErrors* err));
    MOCK_CONST_METHOD4(Read,
                       size_t(FileDescriptor fd, void* buf, size_t length, IOErrors* err));
    MOCK_CONST_METHOD4(Write,
                       size_t(FileDescriptor fd, const void* buf, size_t length, IOErrors* err));
    MOCK_CONST_METHOD2(CreateNewDirectory,
                       void(const std::string& directory_name, hidra2::IOErrors* err));
    MOCK_CONST_METHOD3(GetDataFromFile,
                       FileData(const std::string& fname, uint64_t fsize, IOErrors* err));
    MOCK_CONST_METHOD3(CollectFileInformationRecursivly,
                       void(const std::string& path, std::vector<FileInfo>* files, IOErrors* err));
    MOCK_CONST_METHOD2(FilesInFolder,
                       std::vector<FileInfo>(const std::string& folder, IOErrors* err));
};

}

#endif //HIDRA2_COMMON__MOCKIO_H
