#ifndef ASAPO_COMMON__MOCKIO_H
#define ASAPO_COMMON__MOCKIO_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "io/io.h"
namespace asapo {
class MockIO : public IO {
  public:

    std::string GetHostName(Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto res = GetHostName_t(&error);
        err->reset(error);
        return res;

    }
    MOCK_CONST_METHOD1(GetHostName_t, std::string(ErrorInterface** err));


    std::string AddressFromSocket(SocketDescriptor socket) const noexcept override {
        return AddressFromSocket_t(socket);
    }
    MOCK_CONST_METHOD1(AddressFromSocket_t, std::string (SocketDescriptor socket));


    std::unique_ptr<std::thread> NewThread(const std::string& name, std::function<void()> function) const override {
        return std::unique_ptr<std::thread>(NewThread_t(function));
    }
    MOCK_CONST_METHOD1(NewThread_t, std::thread * (std::function<void()> function));

    std::unique_ptr<std::thread> NewThread(const std::string& name, std::function<void(uint64_t index)> function,
                                           uint64_t index) const override {
        return std::unique_ptr<std::thread>(NewThread_t(function, index));
    }
    MOCK_CONST_METHOD2(NewThread_t, std::thread * (std::function<void(uint64_t)> function, uint64_t index));


    ListSocketDescriptors WaitSocketsActivity(SocketDescriptor master_socket, ListSocketDescriptors* sockets_to_listen,
                                              std::vector<std::string>* new_connections, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = WaitSocketsActivity_t(master_socket, sockets_to_listen, new_connections, &error);
        err->reset(error);
        return data;
    }

    MOCK_CONST_METHOD4(WaitSocketsActivity_t, ListSocketDescriptors(SocketDescriptor master_socket,
                       ListSocketDescriptors* sockets_to_listen,
                       std::vector<std::string>* connections, ErrorInterface** err));


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

    SocketDescriptor CreateAndBindIPTCPSocketListener(const std::string& address, int backlog, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = CreateAndBindIPTCPSocketListener_t(address, backlog, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD3(CreateAndBindIPTCPSocketListener_t, SocketDescriptor(const std::string& address, int backlog,
                       ErrorInterface** err));


    std::unique_ptr<std::tuple<std::string, SocketDescriptor>> InetAcceptConnection(SocketDescriptor socket_fd,
    Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = InetAcceptConnection_t(socket_fd, &error);
        err->reset(error);
        return std::unique_ptr<std::tuple<std::string, SocketDescriptor>>(data);
    }
    MOCK_CONST_METHOD2(InetAcceptConnection_t, std::tuple<std::string, SocketDescriptor>* (SocketDescriptor socket_fd,
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

    size_t ReceiveWithTimeout(SocketDescriptor socket_fd, void* buf, size_t length, long timeout_in_usec,
                              Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = ReceiveWithTimeout_t(socket_fd, buf, length, timeout_in_usec, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD5(ReceiveWithTimeout_t, size_t(SocketDescriptor socket_fd, void* buf, size_t length,
                                                    long timeout_in_usec,
                                                    ErrorInterface** err));

    size_t Send(SocketDescriptor socket_fd, const void* buf, size_t length, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = Send_t(socket_fd, buf, length, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD4(Send_t, size_t(SocketDescriptor socket_fd, const void* buf, size_t length, ErrorInterface** err));

    std::unique_ptr<std::tuple<std::string, uint16_t>> SplitAddressToHostnameAndPort(const std::string& address) const
    override {
        return std::unique_ptr<std::tuple<std::string, uint16_t>>(SplitAddressToHostnameAndPort_t(address));
    }
    MOCK_CONST_METHOD1(SplitAddressToHostnameAndPort_t, std::tuple<std::string, uint16_t>* (const std::string& address));

    void Skip(SocketDescriptor socket_fd, size_t length, Error* err) const override {
        ErrorInterface* error = nullptr;
        Skip_t(socket_fd, length, &error);
        err->reset(error);
    }
    MOCK_CONST_METHOD3(Skip_t, void(SocketDescriptor socket_fd, size_t length, ErrorInterface** err));

    void CloseSocket(SocketDescriptor socket_fd, Error* err) const override {
        ErrorInterface* error = nullptr;
        CloseSocket_t(socket_fd, &error);
        if(err) {
            err->reset(error);
        }
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
        if(err) {
            err->reset(error);
        };
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

    void CreateNewDirectory(const std::string& directory_name, asapo::Error* err) const override {
        ErrorInterface* error = nullptr;
        CreateNewDirectory_t(directory_name, &error);
        err->reset(error);
    }
    MOCK_CONST_METHOD2(CreateNewDirectory_t, void(const std::string& directory_name, ErrorInterface** err));

    FileData GetDataFromFile(const std::string& fname, uint64_t* fsize, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = GetDataFromFile_t(fname, fsize, &error);
        err->reset(error);
        return FileData(data);
    }

    MOCK_CONST_METHOD3(GetDataFromFile_t, uint8_t* (const std::string& fname, uint64_t* fsize, ErrorInterface** err));


    Error GetLastError() const override {
        return Error{GetLastError_t()};
    }

    MOCK_CONST_METHOD0(GetLastError_t, ErrorInterface * ());

    Error SendFile(SocketDescriptor socket_fd, const std::string& fname, size_t length) const override {
        return Error{SendFile_t(socket_fd, fname, length)};
    }
    MOCK_CONST_METHOD3(SendFile_t, ErrorInterface * (SocketDescriptor socket_fd, const std::string& fname, size_t length));

    Error WriteDataToFile(const std::string& root_folder, const std::string& fname, const FileData& data,
                          size_t length, bool create_directories, bool allow_ovewrite) const override {
        return Error{WriteDataToFile_t(root_folder, fname, data.get(), length, create_directories, allow_ovewrite)};

    }

    MOCK_CONST_METHOD1(RemoveFile_t, ErrorInterface * (const std::string& fname));

    Error WriteDataToFile(const std::string& root_folder, const std::string& fname, const uint8_t* data,
                          size_t length, bool create_directories, bool allow_ovewrite) const override {
        return Error{WriteDataToFile_t(root_folder, fname, data, length, create_directories, allow_ovewrite)};
    }


    Error RemoveFile(const std::string& fname) const override {
        return Error{RemoveFile_t(fname)};
    }


    MOCK_CONST_METHOD6(ReceiveDataToFile_t, ErrorInterface * (SocketDescriptor socket, const std::string& root_folder,
                       const std::string& fname, size_t fsize, bool create_directories, bool allow_ovewrite));

    Error ReceiveDataToFile(SocketDescriptor socket, const std::string& root_folder, const std::string& fname,
                            size_t length, bool create_directories, bool allow_ovewrite) const override {
        return Error{ReceiveDataToFile_t(socket, root_folder, fname, length, create_directories, allow_ovewrite)};
    }


    MOCK_CONST_METHOD6(WriteDataToFile_t, ErrorInterface * (const std::string& root_folder, const std::string& fname,
                       const uint8_t* data, size_t fsize, bool create_directories, bool allow_ovewrite));


    FileInfo GetFileInfo(const std::string& name, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = GetFileInfo_t(name, &error);
        err->reset(error);
        return data;

    }

    MOCK_CONST_METHOD2(GetFileInfo_t, FileInfo (const std::string& name, ErrorInterface** err));

    std::vector<FileInfo> FilesInFolder(const std::string& folder, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = FilesInFolder_t(folder, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD2(FilesInFolder_t, std::vector<FileInfo>(const std::string& folder, ErrorInterface** err));


    SubDirList GetSubDirectories(const std::string& path, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = GetSubDirectories_t(path, &error);
        err->reset(error);
        return data;
    };

    MOCK_CONST_METHOD2(GetSubDirectories_t, SubDirList(const std::string& path, ErrorInterface** err));


    std::string ReadFileToString(const std::string& fname, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = ReadFileToString_t(fname, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD2(ReadFileToString_t, std::string(const std::string& fname, ErrorInterface** err));



};

}

#endif //ASAPO_COMMON__MOCKIO_H
