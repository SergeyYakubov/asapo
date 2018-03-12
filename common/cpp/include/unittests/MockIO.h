#ifndef HIDRA2_COMMON__MOCKIO_H
#define HIDRA2_COMMON__MOCKIO_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "system_wrappers/io.h"
namespace hidra2 {
class MockIO : public IO {
  public:
    std::unique_ptr<std::thread> NewThread(std::function<void()> function) const override {
        return std::unique_ptr<std::thread>(NewThread_t(function));
    }
    MOCK_CONST_METHOD1(NewThread_t, std::thread * (std::function<void()> function));

    SocketDescriptor CreateSocket(AddressFamilies address_family, SocketTypes socket_type, SocketProtocols socket_protocol,
                                  Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = CreateSocket_t(address_family, socket_type, socket_protocol, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD4(CreateSocket_t, SocketDescriptor(AddressFamilies address_family, SocketTypes socket_type,
                       SocketProtocols socket_protocol, ErrorInterface** err));

    void Listen(SocketDescriptor socket_fd, int backlog, Error* err) const override {
        ErrorInterface* error = nullptr;
        Listen_t(socket_fd, backlog, &error);
        err->reset(error);
    }
    MOCK_CONST_METHOD3(Listen_t, void(SocketDescriptor socket_fd, int backlog, ErrorInterface** err));


    void InetBind(SocketDescriptor socket_fd, const std::string& address, Error* err) const override {
        ErrorInterface* error = nullptr;
        InetBind_t(socket_fd, address, &error);
        err->reset(error);
    }
    MOCK_CONST_METHOD3(InetBind_t, void(SocketDescriptor socket_fd, const std::string& address, ErrorInterface** err));


    std::unique_ptr<std::tuple<std::string, SocketDescriptor>> InetAccept(SocketDescriptor socket_fd, Error* err) const {
        ErrorInterface* error = nullptr;
        auto data = InetAccept_t(socket_fd, &error);
        err->reset(error);
        return std::unique_ptr<std::tuple<std::string, SocketDescriptor>>(data);
    }
    MOCK_CONST_METHOD2(InetAccept_t, std::tuple<std::string, SocketDescriptor>* (SocketDescriptor socket_fd,
                       ErrorInterface** err));

    std::string ResolveHostnameToIp(const std::string& hostname, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = ResolveHostnameToIp_t(hostname, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD2(ResolveHostnameToIp_t, std::string(const std::string& hostname, ErrorInterface** err));

    void InetConnect(SocketDescriptor socket_fd, const std::string& address, Error* err) const override {
        ErrorInterface* error = nullptr;
        InetConnect_t(socket_fd, address, &error);
        err->reset(error);
    }
    MOCK_CONST_METHOD3(InetConnect_t, void(SocketDescriptor socket_fd, const std::string& address, ErrorInterface** err));

    SocketDescriptor CreateAndConnectIPTCPSocket(const std::string& address, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = CreateAndConnectIPTCPSocket_t(address, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD2(CreateAndConnectIPTCPSocket_t, SocketDescriptor(const std::string& address, ErrorInterface** err));

    size_t Receive(SocketDescriptor socket_fd, void* buf, size_t length, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = Receive_t(socket_fd, buf, length, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD4(Receive_t, size_t(SocketDescriptor socket_fd, void* buf, size_t length, ErrorInterface** err));

    size_t ReceiveTimeout(SocketDescriptor socket_fd, void* buf, size_t length, long timeout_in_usec,
                          Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = ReceiveTimeout_t(socket_fd, buf, length, timeout_in_usec, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD5(ReceiveTimeout_t, size_t(SocketDescriptor socket_fd, void* buf, size_t length, long timeout_in_usec,
                                                ErrorInterface** err));

    size_t Send(SocketDescriptor socket_fd, const void* buf, size_t length, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = Send_t(socket_fd, buf, length, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD4(Send_t, size_t(SocketDescriptor socket_fd, const void* buf, size_t length, ErrorInterface** err));

    void Skip(SocketDescriptor socket_fd, size_t length, Error* err) const override {
        ErrorInterface* error = nullptr;
        Skip_t(socket_fd, length, &error);
        err->reset(error);
    }
    MOCK_CONST_METHOD3(Skip_t, void(SocketDescriptor socket_fd, size_t length, ErrorInterface** err));

    void CloseSocket(SocketDescriptor socket_fd, Error* err) const override {
        ErrorInterface* error = nullptr;
        CloseSocket_t(socket_fd, &error);
        err->reset(error);
    }
    MOCK_CONST_METHOD2(CloseSocket_t, void(SocketDescriptor socket_fd, ErrorInterface** err));

    FileDescriptor Open(const std::string& filename, int open_flags, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = Open_t(filename, open_flags, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD3(Open_t, FileDescriptor(const std::string& filename, int open_flags, ErrorInterface** err));

    void Close(FileDescriptor fd, Error* err) const override {
        ErrorInterface* error = nullptr;
        Close_t(fd, &error);
        err->reset(error);
    }
    MOCK_CONST_METHOD2(Close_t, void(FileDescriptor fd, ErrorInterface** err));

    size_t Read(FileDescriptor fd, void* buf, size_t length, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = Read_t(fd, buf, length, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD4(Read_t, size_t(FileDescriptor fd, void* buf, size_t length, ErrorInterface** err));

    size_t Write(FileDescriptor fd, const void* buf, size_t length, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = Write_t(fd, buf, length, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD4(Write_t, size_t(FileDescriptor fd, const void* buf, size_t length, ErrorInterface** err));

    void CreateNewDirectory(const std::string& directory_name, hidra2::Error* err) const override {
        ErrorInterface* error = nullptr;
        CreateNewDirectory_t(directory_name, &error);
        err->reset(error);
    }
    MOCK_CONST_METHOD2(CreateNewDirectory_t, void(const std::string& directory_name, ErrorInterface** err));

    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = GetDataFromFile_t(fname, fsize, &error);
        err->reset(error);
        return FileData(data);
    }
    MOCK_CONST_METHOD3(GetDataFromFile_t, uint8_t* (const std::string& fname, uint64_t fsize, ErrorInterface** err));

    void CollectFileInformationRecursively(const std::string& path, std::vector<FileInfo>* files,
                                           Error* err) const override {
        ErrorInterface* error = nullptr;
        CollectFileInformationRecursivly_t(path, files, &error);
        err->reset(error);
    }
    MOCK_CONST_METHOD3(CollectFileInformationRecursivly_t, void(const std::string& path, std::vector<FileInfo>* files,
                       ErrorInterface** err));

    std::vector<FileInfo> FilesInFolder(const std::string& folder, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = FilesInFolder_t(folder, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD2(FilesInFolder_t, std::vector<FileInfo>(const std::string& folder, ErrorInterface** err));

    std::string ReadFileToString(const std::string& fname, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = ReadFileToString_t(fname, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD2(ReadFileToString_t, std::string(const std::string& fname, ErrorInterface** err));
};

}

#endif //HIDRA2_COMMON__MOCKIO_H
