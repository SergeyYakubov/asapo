#include <fcntl.h>
#include <iostream>
#include <fstream>      // std::ifstream
#include <sstream>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <mutex>

#if defined(__linux__) || defined (__APPLE__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <sys/select.h>
#endif

#include "system_io.h"
#include "preprocessor/definitions.h"

namespace asapo {

const int SystemIO::kNetBufferSize = 1024 * 1024;
const int SystemIO::kWaitTimeoutMs = 1000;
const size_t SystemIO::kMaxTransferChunkSize = size_t(1024) * size_t(1024) * size_t(1024) * size_t(2); //2GiByte
const size_t SystemIO::kReadWriteBufSize = size_t(1024) * 1024 * 50; //50MiByte



/*******************************************************************************
 *                              system_io.cpp                                  *
 * THIS FILE HOLDS GENERAL FUNCTIONS THAT CAN BE USED ON WINDOWS AND ON LINUX  *
 *******************************************************************************/

// PRIVATE FUNCTIONS - START

void SortFileList(std::vector<FileInfo>* file_list) {
    std::sort(file_list->begin(), file_list->end(),
    [](FileInfo const & a, FileInfo const & b) {
        return a.timestamp < b.timestamp;
    });
}

void StripBasePath(const std::string& folder, std::vector<FileInfo>* file_list) {
    auto n_erase = folder.size() + 1;
    for (auto& file : *file_list) {
        file.name.erase(0, n_erase);
    }
}

void AssignIDs(FileInfos* file_list) {
    int64_t id = 0;
    for (auto& file : *file_list) {
        file.id = ++id;
    }
}

std::unique_ptr<std::tuple<std::string, uint16_t>> SystemIO::SplitAddressToHostnameAndPort(
const std::string& address) const {
    try {
        std::string host = address.substr(0, address.find(':'));

        std::string port_str = address.substr(address.find(':') + 1, address.length());
        uint16_t port = static_cast<uint16_t>(std::stoi(port_str));

        return std::unique_ptr<std::tuple<std::string, uint16_t>>(new std::tuple<std::string, uint16_t>(host, port));
    } catch (...) {
        return nullptr;
    }
}

uint8_t* SystemIO::AllocateArray(uint64_t fsize, Error* err) const {
    uint8_t* data_array = nullptr;
    try {
        data_array = new uint8_t[(size_t)fsize];
    } catch (...) {
        *err = ErrorTemplates::kMemoryAllocationError.Generate();
        return nullptr;
    }
    return data_array;
}

// PRIVATE FUNCTIONS - END

FileData SystemIO::GetDataFromFile(const std::string& fname, uint64_t* fsize, Error* err) const {

    if (*fsize == 0 && !fname.empty()) {
        auto info = GetFileInfo(fname, err);
        if (*err != nullptr) {
            return nullptr;
        }
        *fsize = info.size;
    }

    *err = nullptr;
    auto fd = Open(fname, IO_OPEN_MODE_READ, err);
    if (*err != nullptr) {
        return nullptr;
    }

    auto data_array = AllocateArray(*fsize, err);
    if (*err != nullptr) {
        return nullptr;
    }

    Read(fd, data_array, (size_t)*fsize, err);
    if (*err != nullptr) {
        (*err)->Append(fname + ", expected size: " + std::to_string(*fsize));
        Close(fd, nullptr);
        return nullptr;
    }

    Close(fd, err);
    return FileData{data_array};
}

FileInfos SystemIO::FilesInFolder(const std::string& folder, Error* err) const {
    FileInfos files{};
    CollectFileInformationRecursively(folder, &files, err);
    if (*err != nullptr) {
        return {};
    }
    StripBasePath(folder, &files);
    SortFileList(&files);
    AssignIDs(&files);
    return files;
}

SubDirList SystemIO::GetSubDirectories(const std::string& path, Error* err) const {
    SubDirList list;
    GetSubDirectoriesRecursively(path, &list, err);
    if (*err != nullptr) {
        return {};
    }
    return list;
}

void asapo::SystemIO::CreateNewDirectory(const std::string& directory_name, Error* err) const {
    if (_mkdir(directory_name.c_str()) == -1) {
        *err = GetLastError();
    } else {
        *err = nullptr;
    }
}

FileDescriptor SystemIO::OpenWithCreateFolders(const std::string& root_folder, const std::string& fname,
                                               bool create_directories, bool allow_ovewrite, Error* err) const {
    std::string full_name;
    if (!root_folder.empty()) {
        full_name = root_folder + kPathSeparator + fname;
    } else {
        full_name = fname;
    }
    auto create_flag = allow_ovewrite ? IO_OPEN_MODE_CREATE : IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS;
    auto fd = Open(full_name, create_flag | IO_OPEN_MODE_RW | IO_OPEN_MODE_SET_LENGTH_0, err);
    if (*err == IOErrorTemplates::kFileNotFound && create_directories)  {
        size_t pos = fname.rfind(kPathSeparator);
        if (pos == std::string::npos) {
            *err = IOErrorTemplates::kFileNotFound.Generate(full_name);
            return -1;
        }
        *err = CreateDirectoryWithParents(root_folder, fname.substr(0, pos));
        if (*err) {
            return -1;
        }
        return OpenWithCreateFolders(root_folder, fname, false, allow_ovewrite, err);
    }

    return fd;

}

Error SystemIO::WriteDataToFile(const std::string& root_folder, const std::string& fname, const uint8_t* data,
                                size_t length, bool create_directories, bool allow_ovewrite) const {
    Error err;
    auto fd = OpenWithCreateFolders(root_folder, fname, create_directories, allow_ovewrite, &err);
    if (err) {
        return err;
    }

    Write(fd, data, length, &err);
    if (err) {
        err->Append(fname);
        return err;
    }

    Close(fd, &err);
    return err;

}

Error SystemIO::WriteDataToFile(const std::string& root_folder, const std::string& fname, const FileData& data,
                                size_t length, bool create_directories, bool allow_ovewrite) const {
    return WriteDataToFile(root_folder, fname, data.get(), length, create_directories, allow_ovewrite);
}

std::string SystemIO::ReadFileToString(const std::string& fname, Error* err) const {

    uint64_t size = 0;
    auto data = GetDataFromFile(fname, &size, err);
    if (*err != nullptr) {
        return "";
    }

    return std::string(reinterpret_cast<const char*>(data.get()), (size_t)size);
}

std::unique_ptr<std::thread> SystemIO::NewThread(const std::string& name, std::function<void()> function) const {
    auto thread = std::unique_ptr<std::thread>(new std::thread(function));
    SetThreadName(thread.get(), name);
    return thread;
}

std::unique_ptr<std::thread> SystemIO::NewThread(const std::string& name, std::function<void(uint64_t index)> function,
                                                 uint64_t index) const {
    auto thread = std::unique_ptr<std::thread>(new std::thread(function, index));
    SetThreadName(thread.get(), name + ":" + std::to_string(index));
    return thread;
}

void SystemIO::Skip(SocketDescriptor socket_fd, size_t length, Error* err) const {
    static const size_t kSkipBufferSize = 1024;

    //TODO need to find a better way to skip bytes
    *err = nullptr;
    std::unique_ptr<uint8_t[]> buffer;
    try {
        buffer.reset(new uint8_t[kSkipBufferSize]);
    } catch (...) {
        *err = ErrorTemplates::kMemoryAllocationError.Generate();
        return;
    }
    size_t already_skipped = 0;
    while (already_skipped < length) {
        size_t need_to_skip = length - already_skipped;
        if (need_to_skip > kSkipBufferSize)
            need_to_skip = kSkipBufferSize;
        size_t skipped_amount = Receive(socket_fd, buffer.get(), need_to_skip, err);
        if (*err != nullptr) {
            return;
        }
        already_skipped += skipped_amount;
    }
}

asapo::FileDescriptor asapo::SystemIO::CreateAndConnectIPTCPSocket(const std::string& address,
        Error* err) const {
    *err = nullptr;

    FileDescriptor fd = CreateSocket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP, err);
    if (*err != nullptr) {
        return kDisconnectedSocketDescriptor;
    }

    InetConnect(fd, address, err);
    if (*err != nullptr) {
        CloseSocket(fd, nullptr);
        return kDisconnectedSocketDescriptor;
    }

    return fd;
}

int SystemIO::FileOpenModeToPosixFileOpenMode(int open_flags) const {
    int flags = 0;
    if (((open_flags & IO_OPEN_MODE_READ) && (open_flags & IO_OPEN_MODE_WRITE)) || (open_flags & IO_OPEN_MODE_RW)) {
        flags |= O_RDWR;
    } else {
        if (open_flags & IO_OPEN_MODE_READ) {
            flags |= O_RDONLY;
        }
        if (open_flags & IO_OPEN_MODE_WRITE) {
            flags |= O_WRONLY;
        }
    }
    if (open_flags & IO_OPEN_MODE_CREATE) {
        flags |= O_CREAT;
    }
    if (open_flags & IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS) {
        flags |= O_CREAT | O_EXCL;
    }
    if (open_flags & IO_OPEN_MODE_SET_LENGTH_0) {
        flags |= O_TRUNC;
    }
    return flags;
}

std::string SystemIO::ResolveHostnameToIp(const std::string& hostname, Error* err) const {
    static std::mutex lock;
    std::unique_lock<std::mutex> local_lock(lock);

    const hostent* record = gethostbyname(hostname.c_str()); // gethostbyname seems not to be thread safe!
    if (record == nullptr || record->h_addr == nullptr) {
        *err = IOErrorTemplates::kUnableToResolveHostname.Generate();
        return "";
    }
    in_addr* address = (in_addr*) (record->h_addr);
    std::string ip_address = inet_ntoa(*address);

    *err = nullptr;
    return ip_address;
}

std::unique_ptr<sockaddr_in> SystemIO::BuildSockaddrIn(const std::string& address, Error* err) const {
    auto hostname_port_tuple = SplitAddressToHostnameAndPort(address);
    if (!hostname_port_tuple) {
        *err = IOErrorTemplates::kInvalidAddressFormat.Generate();
        return nullptr;
    }
    std::string host;
    uint16_t port = 0;
    std::tie(host, port) = *hostname_port_tuple;

    host = ResolveHostnameToIp(host, err);
    if (*err != nullptr) {
        return nullptr;
    }

    short family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if (family == -1) {
        *err = IOErrorTemplates::kUnsupportedAddressFamily.Generate();
        return nullptr;
    }

    std::unique_ptr<sockaddr_in> socket_address = std::unique_ptr<sockaddr_in>(new sockaddr_in);
    socket_address->sin_addr.s_addr = inet_addr(host.c_str());
    socket_address->sin_port = htons(port);
    socket_address->sin_family = family;

    return socket_address;
}

void asapo::SystemIO::InetConnect(SocketDescriptor socket_fd, const std::string& address, Error* err) const {
    auto socket_address = BuildSockaddrIn(address, err);
    if (*err != nullptr) {
        return;
    }

    if (_connect(socket_fd, socket_address.get(), sizeof(sockaddr_in)) == -1) {
        *err = GetLastError();
        // On windows its normal that connect might give an "WSAEWOULDBLOCK" error,
        // since the socket need time to be created
        if (*err != nullptr && IOErrorTemplates::kResourceTemporarilyUnavailable != *err) {
            return;
        }
    }
    *err = nullptr;
}

std::unique_ptr<std::tuple<std::string, SocketDescriptor>> SystemIO::InetAcceptConnection(SocketDescriptor socket_fd,
Error* err) const {
    static short family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if (family == -1) {
        *err = IOErrorTemplates::kUnsupportedAddressFamily.Generate();
        return nullptr;
    }

    sockaddr_in client_address{};
    static size_t client_address_size = sizeof(sockaddr_in);

    int peer_fd;
    while (true) {
        peer_fd = _accept(socket_fd, reinterpret_cast<sockaddr*>(&client_address), &client_address_size);

        if (peer_fd == -1) {
            *err = GetLastError();
            if (*err != nullptr && IOErrorTemplates::kResourceTemporarilyUnavailable != *err) {
                continue;
            }
            return nullptr;
        }
        break;
    }

    *err = nullptr;
    ApplyNetworkOptions(peer_fd, err);

    std::string
    address = std::string(inet_ntoa(client_address.sin_addr)) + ':' + std::to_string(client_address.sin_port);
    return std::unique_ptr<std::tuple<std::string, SocketDescriptor>>(new
            std::tuple<std::string,
            SocketDescriptor>(
                address,
                peer_fd));
}

asapo::FileDescriptor asapo::SystemIO::Open(const std::string& filename,
                                            int open_flags,
                                            Error* err) const {
    int flags = FileOpenModeToPosixFileOpenMode(open_flags);
    FileDescriptor fd = _open(filename.c_str(), flags);
    if (fd == -1) {
        *err = GetLastError();
        (*err)->Append(filename);
    } else {
        *err = nullptr;
    }
    return fd;
}

void asapo::SystemIO::Close(FileDescriptor fd, Error* err) const {
    if (err) {
        *err = nullptr;
    }
    if (!_close(fd) && err) {
        *err = GetLastError();
    }
}

short asapo::SystemIO::AddressFamilyToPosixFamily(AddressFamilies address_family) const {
    switch (address_family) {
    case AddressFamilies::INET:
        return AF_INET;
    }
    return -1;
}

int asapo::SystemIO::SocketTypeToPosixType(SocketTypes socket_type) const {
    switch (socket_type) {
    case SocketTypes::STREAM:
        return SOCK_STREAM;
    }
    return -1;
}

int asapo::SystemIO::SocketProtocolToPosixProtocol(SocketProtocols socket_protocol) const {
    switch (socket_protocol) {
    case SocketProtocols::IP:
        return IPPROTO_IP;
    }
    return -1;
}

SocketDescriptor SystemIO::CreateSocket(AddressFamilies address_family,
                                        SocketTypes socket_type,
                                        SocketProtocols socket_protocol,
                                        Error* err) const {
    int domain = AddressFamilyToPosixFamily(address_family);
    if (domain == -1) {
        *err = IOErrorTemplates::kUnsupportedAddressFamily.Generate();
        return -1;
    }

    int type = SocketTypeToPosixType(socket_type);
    if (type == -1) {
        *err = IOErrorTemplates::kUnknownIOError.Generate();
        return -1;
    }

    int protocol = SocketProtocolToPosixProtocol(socket_protocol);
    if (protocol == -1) {
        *err = IOErrorTemplates::kUnknownIOError.Generate();
        return -1;
    }

    SocketDescriptor socket_fd = _socket(domain, type, protocol);
    if (socket_fd == -1) {
        *err = GetLastError();
        return socket_fd;
    }

    *err = nullptr;

    ApplyNetworkOptions(socket_fd, err);

    return socket_fd;
}

void asapo::SystemIO::InetBind(SocketDescriptor socket_fd, const std::string& address,
                               Error* err) const {
    *err = nullptr;

    int family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if (family == -1) {
        *err = IOErrorTemplates::kUnsupportedAddressFamily.Generate();
        return;
    }

    auto socket_address = BuildSockaddrIn(address, err);
    if (*err != nullptr) {
        return;
    }

    if (::bind(socket_fd, reinterpret_cast<const sockaddr*>(socket_address.get()), sizeof(sockaddr_in)) == -1) {
        *err = GetLastError();
    }
}

void asapo::SystemIO::Listen(SocketDescriptor socket_fd, int backlog, Error* err) const {
    *err = nullptr;

    if (_listen(socket_fd, backlog) == -1) {
        *err = GetLastError();
    }
}

SocketDescriptor SystemIO::CreateAndBindIPTCPSocketListener(const std::string& address, int backlog, Error* err) const {
    FileDescriptor listener_fd = CreateSocket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP, err);

    if (*err) {
        return -1;
    }

    InetBind(listener_fd, address, err);
    if (*err) {
        CloseSocket(listener_fd, nullptr);
        return -1;
    }

    Listen(listener_fd, backlog, err);
    if (*err) {
        CloseSocket(listener_fd, nullptr);
        return -1;
    }

    return listener_fd;
}

size_t asapo::SystemIO::ReceiveWithTimeout(SocketDescriptor socket_fd, void* buf, size_t length, long timeout_in_usec,
                                           Error* err) const {
    *err = nullptr;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(socket_fd, &read_fds);
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_in_usec;

    int res = ::select(socket_fd + 1, &read_fds, nullptr, nullptr, &timeout);
    if (res == 0) {
        *err = IOErrorTemplates::kTimeout.Generate();
        return 0;
    }
    if (res == -1) {
        *err = GetLastError();
        return 0;
    }

    return Receive(socket_fd, buf, length, err);
}

size_t asapo::SystemIO::Read(FileDescriptor fd, void* buf, size_t length, Error* err) const {
    return Transfer(_read, fd, buf, length, err);
}

size_t asapo::SystemIO::Write(FileDescriptor fd, const void* buf, size_t length, Error* err) const {
    return Transfer(_write, fd, buf, length, err);
}

size_t asapo::SystemIO::Receive(SocketDescriptor socket_fd, void* buf, size_t length, Error* err) const {
    return Transfer(_recv, socket_fd, buf, length, err);
}

size_t asapo::SystemIO::Send(SocketDescriptor socket_fd, const void* buf, size_t length, Error* err) const {
    return Transfer(_send, socket_fd, buf, length, err);
}

size_t SystemIO::Transfer(ssize_t (* method)(FileDescriptor, const void*, size_t),
                          FileDescriptor fd,
                          const void* buf,
                          size_t length,
                          Error* err) const {
    return Transfer(reinterpret_cast<ssize_t (*)(FileDescriptor, void*, size_t)>(method), fd,
                    const_cast<void*>(buf), length, err);
}

size_t SystemIO::Transfer(ssize_t (* method)(FileDescriptor, void*, size_t), FileDescriptor fd, void* buf,
                          size_t length, Error* err) const {
    *err = nullptr;
    size_t already_transferred = 0;

    while (already_transferred < length) {
        ssize_t received_amount = method(fd, (uint8_t*) buf + already_transferred,
                                         std::min(kMaxTransferChunkSize, length - already_transferred));
        if (received_amount == 0) {
            *err = ErrorTemplates::kEndOfFile.Generate();
            return already_transferred;
        }
        if (received_amount == -1) {
            *err = GetLastError();
            if (IOErrorTemplates::kResourceTemporarilyUnavailable == *err) {
                continue;
            }
            if (*err == nullptr) {
                *err = IOErrorTemplates::kUnknownIOError.Generate();
            }
            return already_transferred;//Return the amount of _ensured_ transferred bytes
        }
        already_transferred += received_amount;
    }
    *err = nullptr;
    return already_transferred;
}

Error SystemIO::CreateDirectoryWithParents(const std::string& root_path, const std::string& path) const {
    for (std::string::const_iterator iter = path.begin(); iter != path.end();) {
        iter = std::find(iter, path.end(), kPathSeparator);
        std::string new_path;
        if (root_path.empty()) {
            new_path = std::string(path.begin(), iter);
        } else {
            new_path = root_path + kPathSeparator + std::string(path.begin(), iter);
        }
        Error err;
        CreateNewDirectory(new_path, &err);
        if (err && err != IOErrorTemplates::kFileAlreadyExists) {
            err->Append(new_path);
            return err;
        }
        if (iter != path.end()) {
            ++iter;
        }
    }
    return nullptr;
}

Error SystemIO::RemoveFile(const std::string& fname) const {
    if (remove(fname.c_str()) == 0) {
        return nullptr;;
    } else {
        return GetLastError();
    }
}


std::string SystemIO::GetHostName(Error* err) const noexcept {
    char host[1024];
    gethostname(host, sizeof(host));
    *err = GetLastError();
    if (*err) {
        return "";
    } else {
        return host;
    }
}

Error SystemIO::SendFile(SocketDescriptor socket_fd, const std::string& fname, size_t length) const {

    size_t total_bytes_sent = 0;

    size_t buf_size = std::min(length, kReadWriteBufSize);

    Error err;
    auto fd = Open(fname, IO_OPEN_MODE_READ, &err);
    if (err != nullptr) {
        return err;
    }

    auto data_array = std::unique_ptr<uint8_t> {AllocateArray(buf_size, &err)};
    if (err != nullptr) {
        return err;
    }

    while (total_bytes_sent < length) {
        auto bytes_read = Read(fd, data_array.get(), buf_size, &err);
        if (err != nullptr && err != ErrorTemplates::kEndOfFile) {
            Close(fd, nullptr);
            return err;
        }
        auto bytes_sent = Send(socket_fd, data_array.get(), bytes_read, &err);
        if (err != nullptr) {
            Close(fd, nullptr);
            return err;
        }
        total_bytes_sent += bytes_sent;
    }

    Close(fd, nullptr);
    return nullptr;
}

Error SystemIO:: ReceiveDataToFile(SocketDescriptor socket, const std::string& root_folder, const std::string& fname,
                                   size_t length, bool create_directories, bool allow_ovewrite) const {
    Error err;
    auto fd = OpenWithCreateFolders(root_folder, fname, create_directories, allow_ovewrite, &err);
    if (err) {
        return err;
    }

    size_t buf_size = std::min(length, kReadWriteBufSize);
    auto data_array = std::unique_ptr<uint8_t> {AllocateArray(buf_size, &err)};
    if (err != nullptr) {
        return err;
    }

    size_t total_bytes_written = 0;
    while (total_bytes_written < length) {
        auto bytes_received = Receive(socket, data_array.get(), std::min(buf_size, length - total_bytes_written), &err);
        if (err != nullptr && err != ErrorTemplates::kEndOfFile) {
            Close(fd, nullptr);
            return err;
        }
        auto bytes_written = Write(fd, data_array.get(), bytes_received, &err);
        if (err != nullptr) {
            Close(fd, nullptr);
            return err;
        }
        total_bytes_written += bytes_written;
    }

    Close(fd, nullptr);
    return nullptr;
}



}
