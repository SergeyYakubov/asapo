#ifndef ASAPO_COMMON__MOCKIO_H
#define ASAPO_COMMON__MOCKIO_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/io/io.h"
namespace asapo {
class MockIO : public IO {
  public:

    std::string GetHostName(Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto res = GetHostName_t(&error);
        err->reset(error);
        return res;

    }
    MOCK_METHOD(std::string, GetHostName_t, (ErrorInterface** err), (const));


    std::string AddressFromSocket(SocketDescriptor socket) const noexcept override {
        return AddressFromSocket_t(socket);
    }
    MOCK_METHOD(std::string, AddressFromSocket_t, (SocketDescriptor socket), (const));


    std::unique_ptr<std::thread> NewThread(const std::string&, std::function<void()> function) const override {
        return std::unique_ptr<std::thread>(NewThread_t(function));
    }
    MOCK_METHOD(std::thread *, NewThread_t, (std::function<void()> function), (const));

    std::unique_ptr<std::thread> NewThread(const std::string&, std::function<void(uint64_t index)> function,
                                           uint64_t index) const override {
        return std::unique_ptr<std::thread>(NewThread_t(function, index));
    }
    MOCK_METHOD(std::thread *, NewThread_t, (std::function<void(uint64_t)> function, uint64_t index), (const));


    ListSocketDescriptors WaitSocketsActivity(SocketDescriptor master_socket, ListSocketDescriptors* sockets_to_listen,
                                              std::vector<std::string>* new_connections, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = WaitSocketsActivity_t(master_socket, sockets_to_listen, new_connections, &error);
        err->reset(error);
        return data;
    }

    MOCK_METHOD(ListSocketDescriptors, WaitSocketsActivity_t, (SocketDescriptor master_socket, ListSocketDescriptors* sockets_to_listen, std::vector<std::string>* connections, ErrorInterface** err), (const));


    SocketDescriptor CreateSocket(AddressFamilies address_family, SocketTypes socket_type, SocketProtocols socket_protocol,
                                  Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = CreateSocket_t(address_family, socket_type, socket_protocol, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD(SocketDescriptor, CreateSocket_t, (AddressFamilies address_family, SocketTypes socket_type, SocketProtocols socket_protocol, ErrorInterface** err), (const));

    void Listen(SocketDescriptor socket_fd, int backlog, Error* err) const override {
        ErrorInterface* error = nullptr;
        Listen_t(socket_fd, backlog, &error);
        err->reset(error);
    }
    MOCK_METHOD(void, Listen_t, (SocketDescriptor socket_fd, int backlog, ErrorInterface** err), (const));


    void InetBind(SocketDescriptor socket_fd, const std::string& address, Error* err) const override {
        ErrorInterface* error = nullptr;
        InetBind_t(socket_fd, address, &error);
        err->reset(error);
    }
    MOCK_METHOD(void, InetBind_t, (SocketDescriptor socket_fd, const std::string& address, ErrorInterface** err), (const));

    SocketDescriptor CreateAndBindIPTCPSocketListener(const std::string& address, int backlog, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = CreateAndBindIPTCPSocketListener_t(address, backlog, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD(SocketDescriptor, CreateAndBindIPTCPSocketListener_t, (const std::string& address, int backlog, ErrorInterface** err), (const));


    std::unique_ptr<std::tuple<std::string, SocketDescriptor>> InetAcceptConnection(SocketDescriptor socket_fd,
    Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = InetAcceptConnection_t(socket_fd, &error);
        err->reset(error);
        return std::unique_ptr<std::tuple<std::string, SocketDescriptor>>(data);
    }
    MOCK_METHOD((std::tuple<std::string, SocketDescriptor>*), InetAcceptConnection_t, (SocketDescriptor socket_fd, ErrorInterface** err), (const));

    std::string ResolveHostnameToIp(const std::string& hostname, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = ResolveHostnameToIp_t(hostname, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD(std::string, ResolveHostnameToIp_t, (const std::string& hostname, ErrorInterface** err), (const));

    void InetConnect(SocketDescriptor socket_fd, const std::string& address, Error* err) const override {
        ErrorInterface* error = nullptr;
        InetConnect_t(socket_fd, address, &error);
        err->reset(error);
    }
    MOCK_METHOD(void, InetConnect_t, (SocketDescriptor socket_fd, const std::string& address, ErrorInterface** err), (const));

    SocketDescriptor CreateAndConnectIPTCPSocket(const std::string& address, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = CreateAndConnectIPTCPSocket_t(address, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD(SocketDescriptor, CreateAndConnectIPTCPSocket_t, (const std::string& address, ErrorInterface** err), (const));

    size_t Receive(SocketDescriptor socket_fd, void* buf, size_t length, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = Receive_t(socket_fd, buf, length, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD(size_t, Receive_t, (SocketDescriptor socket_fd, void* buf, size_t length, ErrorInterface** err), (const));

    size_t ReceiveWithTimeout(SocketDescriptor socket_fd, void* buf, size_t length, long timeout_in_usec,
                              Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = ReceiveWithTimeout_t(socket_fd, buf, length, timeout_in_usec, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD(size_t, ReceiveWithTimeout_t, (SocketDescriptor socket_fd, void* buf, size_t length, long timeout_in_usec, ErrorInterface** err), (const));

    size_t Send(SocketDescriptor socket_fd, const void* buf, size_t length, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = Send_t(socket_fd, buf, length, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD(size_t, Send_t, (SocketDescriptor socket_fd, const void* buf, size_t length, ErrorInterface** err), (const));

    std::unique_ptr<std::tuple<std::string, uint16_t>> SplitAddressToHostnameAndPort(const std::string& address) const
    override {
        return std::unique_ptr<std::tuple<std::string, uint16_t>>(SplitAddressToHostnameAndPort_t(address));
    }
    MOCK_METHOD((std::tuple<std::string, uint16_t>*), SplitAddressToHostnameAndPort_t, (const std::string& address), (const));

    void Skip(SocketDescriptor socket_fd, size_t length, Error* err) const override {
        ErrorInterface* error = nullptr;
        Skip_t(socket_fd, length, &error);
        err->reset(error);
    }
    MOCK_METHOD(void, Skip_t, (SocketDescriptor socket_fd, size_t length, ErrorInterface** err), (const));

    void CloseSocket(SocketDescriptor socket_fd, Error* err) const override {
        ErrorInterface* error = nullptr;
        CloseSocket_t(socket_fd, &error);
        if(err) {
            err->reset(error);
        }
    }
    MOCK_METHOD(void, CloseSocket_t, (SocketDescriptor socket_fd, ErrorInterface** err), (const));

    FileDescriptor Open(const std::string& filename, int open_flags, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = Open_t(filename, open_flags, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD(FileDescriptor, Open_t, (const std::string& filename, int open_flags, ErrorInterface** err), (const));

    void Close(FileDescriptor fd, Error* err) const override {
        ErrorInterface* error = nullptr;
        Close_t(fd, &error);
        if(err) {
            err->reset(error);
        }
    }
    MOCK_METHOD(void, Close_t, (FileDescriptor fd, ErrorInterface** err), (const));

    size_t Read(FileDescriptor fd, void* buf, size_t length, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = Read_t(fd, buf, length, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD(size_t, Read_t, (FileDescriptor fd, void* buf, size_t length, ErrorInterface** err), (const));

    size_t Write(FileDescriptor fd, const void* buf, size_t length, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = Write_t(fd, buf, length, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD(size_t, Write_t, (FileDescriptor fd, const void* buf, size_t length, ErrorInterface** err), (const));

    void CreateNewDirectory(const std::string& directory_name, asapo::Error* err) const override {
        ErrorInterface* error = nullptr;
        CreateNewDirectory_t(directory_name, &error);
        err->reset(error);
    }
    MOCK_METHOD(void, CreateNewDirectory_t, (const std::string& directory_name, ErrorInterface** err), (const));

    MessageData GetDataFromFile(const std::string& fname, uint64_t* fsize, Error* err) const override {
        std::function<ErrorInterface*()> error;
        auto data = GetDataFromFile_t(fname, fsize, &error);
        if (error != nullptr) {
            err->reset(error());
        } else {
            err->reset(nullptr);
        }
        return MessageData(data);
    }

    MOCK_METHOD(uint8_t*, GetDataFromFile_t, (const std::string& fname, uint64_t* fsize, std::function<ErrorInterface*()>* err_gen), (const));


    Error GetLastError() const override {
        return Error{GetLastError_t()};
    }

    MOCK_METHOD(ErrorInterface *, GetLastError_t, (), (const));

    Error SendFile(SocketDescriptor socket_fd, const std::string& fname, size_t length) const override {
        return Error{SendFile_t(socket_fd, fname, length)};
    }
    MOCK_METHOD(ErrorInterface *, SendFile_t, (SocketDescriptor socket_fd, const std::string& fname, size_t length), (const));

    Error WriteDataToFile(const std::string& root_folder, const std::string& fname, const MessageData& data,
                          size_t length, bool create_directories, bool allow_ovewrite) const override {
        return Error{WriteDataToFile_t(root_folder, fname, data.get(), length, create_directories, allow_ovewrite)};

    }

    MOCK_METHOD(ErrorInterface *, RemoveFile_t, (const std::string& fname), (const));

    Error WriteDataToFile(const std::string& root_folder, const std::string& fname, const uint8_t* data,
                          size_t length, bool create_directories, bool allow_ovewrite) const override {
        return Error{WriteDataToFile_t(root_folder, fname, data, length, create_directories, allow_ovewrite)};
    }


    Error RemoveFile(const std::string& fname) const override {
        return Error{RemoveFile_t(fname)};
    }


    MOCK_METHOD(ErrorInterface *, ReceiveDataToFile_t, (SocketDescriptor socket, const std::string& root_folder, const std::string& fname, size_t fsize, bool create_directories, bool allow_ovewrite), (const));

    Error ReceiveDataToFile(SocketDescriptor socket, const std::string& root_folder, const std::string& fname,
                            size_t length, bool create_directories, bool allow_ovewrite) const override {
        return Error{ReceiveDataToFile_t(socket, root_folder, fname, length, create_directories, allow_ovewrite)};
    }


    MOCK_METHOD(ErrorInterface *, WriteDataToFile_t, (const std::string& root_folder, const std::string& fname, const uint8_t* data, size_t fsize, bool create_directories, bool allow_ovewrite), (const));


    MessageMeta GetMessageMeta(const std::string& name, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = GetMessageMeta_t(name, &error);
        err->reset(error);
        return data;

    }

    MOCK_METHOD(MessageMeta, GetMessageMeta_t, (const std::string& name, ErrorInterface** err), (const));

    std::vector<MessageMeta> FilesInFolder(const std::string& folder, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = FilesInFolder_t(folder, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD(std::vector<MessageMeta>, FilesInFolder_t, (const std::string& folder, ErrorInterface** err), (const));


    SubDirList GetSubDirectories(const std::string& path, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = GetSubDirectories_t(path, &error);
        err->reset(error);
        return data;
    };

    MOCK_METHOD(SubDirList, GetSubDirectories_t, (const std::string& path, ErrorInterface** err), (const));


    std::string ReadFileToString(const std::string& fname, Error* err) const override {
        ErrorInterface* error = nullptr;
        auto data = ReadFileToString_t(fname, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD(std::string, ReadFileToString_t, (const std::string& fname, ErrorInterface** err), (const));



};

}

#endif //ASAPO_COMMON__MOCKIO_H
