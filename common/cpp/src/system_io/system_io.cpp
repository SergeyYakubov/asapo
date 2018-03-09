#include <fcntl.h>
#include <iostream>
#include <fstream>      // std::ifstream
#include <sstream>
#include <cerrno>
#include <cstring>
#include <algorithm>

#include <system_wrappers/system_io.h>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

namespace hidra2 {

const int SystemIO::kNetBufferSize = 1024 * 1024 * 50; //MiByte

/*******************************************************************************
 *                              system_io.cpp                                  *
 * THIS FILE HOLDS GENERAL FUNCTIONS THAT CAN BE USED ON WINDOWS AND ON LINUX  *
 *******************************************************************************/

// PRIVATE FUNCTIONS - START

void SortFileList(std::vector<FileInfo>* file_list) {
    std::sort(file_list->begin(), file_list->end(),
    [](FileInfo const & a, FileInfo const & b) {
        return a.modify_date < b.modify_date;
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

std::unique_ptr<std::tuple<std::string, uint16_t>> SystemIO::SplitAddressToHostnameAndPort(std::string address) const {
    try {
        std::string host = address.substr(0, address.find(':'));

        std::string port_str = address.substr(address.find(':') + 1, address.length());
        uint16_t port = static_cast<uint16_t>(std::stoi(port_str));

        return std::unique_ptr<std::tuple<std::string, uint16_t>>(new std::tuple<std::string, uint16_t>(host, port));
    } catch (...) {
        return nullptr;
    }
}

uint8_t* AllocateArray(uint64_t fsize, Error* err) {
    uint8_t* data_array = nullptr;
    try {
        data_array = new uint8_t[fsize];
    } catch (...) {
        *err = IOErrorTemplate::kMemoryAllocationError.Copy();
        return nullptr;
    }
    return data_array;
}

// PRIVATE FUNCTIONS - END

FileData SystemIO::GetDataFromFile(const std::string& fname, uint64_t fsize, Error* err) const {
    *err = nullptr;
    auto fd = Open(fname, IO_OPEN_MODE_READ, err);
    if (*err != nullptr) {
        return nullptr;
    }

    auto data_array = AllocateArray(fsize, err);
    if (*err != nullptr) {
        return nullptr;
    }

    Read(fd, data_array, fsize, err);
    if (*err != nullptr) {
        Close(fd, nullptr);
        (*err)->Append(fname);
        return nullptr;
    }

    Close(fd, err);
    return FileData{data_array};
}

FileInfos SystemIO::FilesInFolder(const std::string& folder, Error* err) const {
    FileInfos files{};
    CollectFileInformationRecursivly(folder, &files, err);
    if (*err != nullptr) {
        return {};
    }
    StripBasePath(folder, &files);
    SortFileList(&files);
    AssignIDs(&files);
    return files;
}

void hidra2::SystemIO::CreateNewDirectory(const std::string& directory_name, Error* err) const {
    if(_mkdir(directory_name.c_str()) == -1) {
        *err = GetLastError();
    } else {
        *err = nullptr;
    }
}

std::string SystemIO::ReadFileToString(const std::string& fname, Error* err) const {
    auto info = GetFileInfo(fname, err);
    if (*err != nullptr) {
        return "";
    }

    auto data = GetDataFromFile(fname, info.size, err);
    if (*err != nullptr) {
        return "";
    }

    return std::string(reinterpret_cast<const char*>(data.get()), info.size);
}


std::thread* SystemIO::NewThread(std::function<void()> function) const {
    return new std::thread(function);
}

void SystemIO::Skip(SocketDescriptor socket_fd, size_t length, Error* err) const {
    static const size_t kSkipBufferSize = 1024;

    //TODO need to find a better way to skip bytes
    *err = nullptr;
    std::unique_ptr<uint8_t[]> buffer;
    try {
        buffer.reset(new uint8_t[kSkipBufferSize]);
    } catch(...) {
        *err = IOErrorTemplate::kMemoryAllocationError.Copy();
        return;
    }
    size_t already_skipped = 0;
    while(already_skipped < length) {
        size_t need_to_skip = length - already_skipped;
        if(need_to_skip > kSkipBufferSize)
            need_to_skip = kSkipBufferSize;
        size_t skipped_amount = Receive(socket_fd, buffer.get(), need_to_skip, err);
        if(*err != nullptr) {
            return;
        }
        already_skipped += skipped_amount;
    }
}

hidra2::FileDescriptor hidra2::SystemIO::CreateAndConnectIPTCPSocket(const std::string& address,
        Error* err) const {
    *err = nullptr;

    FileDescriptor fd = CreateSocket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP, err);
    if(*err != nullptr) {
        return -1;
    }

    InetConnect(fd, address, err);
    if (*err != nullptr) {
        CloseSocket(fd, nullptr);
        return -1;
    }

    return fd;
}

int SystemIO::FileOpenModeToPosixFileOpenMode(int open_flags) const {
    int flags = 0;
    if((open_flags & IO_OPEN_MODE_READ && open_flags & IO_OPEN_MODE_WRITE) || open_flags & IO_OPEN_MODE_RW) {
        flags |= O_RDWR;
    } else {
        if (open_flags & IO_OPEN_MODE_READ) {
            flags |= O_RDONLY;
        }
        if (open_flags & IO_OPEN_MODE_WRITE) {
            flags |= O_WRONLY;
        }
    }
    if(open_flags & IO_OPEN_MODE_CREATE) {
        flags |= O_CREAT;
    }
    if(open_flags & IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS) {
        flags |= O_CREAT | O_EXCL;
    }
    if(open_flags & IO_OPEN_MODE_SET_LENGTH_0) {
        flags |= O_TRUNC;
    }
    return flags;
}

std::string SystemIO::ResolveHostnameToIp(const std::string& hostname, Error* err) const {
    InitializeSocketIfNecessary();
    hostent* record = gethostbyname(hostname.c_str());
    if (record == nullptr) {
        *err = IOErrorTemplate::kUnableToResolveHostname.Copy();
        return "";
    }
    in_addr* address = (in_addr*)(record->h_addr);
    std::string ip_address = inet_ntoa(*address);

    *err = nullptr;
    return ip_address;
}

void hidra2::SystemIO::InetConnect(SocketDescriptor socket_fd, const std::string& address, Error* err) const {
    auto hostname_port_tuple = SplitAddressToHostnameAndPort(address);
    if (!hostname_port_tuple) {
        *err = IOErrorTemplate::kInvalidAddressFormat.Copy();
        return;
    }
    std::string host;
    uint16_t port = 0;
    std::tie(host, port) = *hostname_port_tuple;
    host = ResolveHostnameToIp(host, err);
    if(*err != nullptr) {
        return;
    }

    short family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if (family == -1) {
        *err = IOErrorTemplate::kUnsupportedAddressFamily.Copy();
        return;
    }

    sockaddr_in socket_address{};
    socket_address.sin_addr.s_addr = inet_addr(host.c_str());
    socket_address.sin_port = htons(port);
    socket_address.sin_family = family;

    if (_connect(socket_fd, (struct sockaddr*) &socket_address, sizeof(socket_address)) == -1) {
        *err = GetLastError();
        // On windows its normal that connect might throw a "WSAEWOULDBLOCK" since the socket need time to be created
        if (*err != nullptr && (*err)->GetErrorType() != ErrorType::kResourceTemporarilyUnavailable) {
            return;
        }
    }
    *err = nullptr;
}

std::unique_ptr<std::tuple<std::string, SocketDescriptor>> SystemIO::InetAccept(SocketDescriptor socket_fd,
Error* err) const {
    static short family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if (family == -1) {
        *err = IOErrorTemplate::kUnsupportedAddressFamily.Copy();
        return nullptr;
    }

    sockaddr_in client_address{};
    static size_t client_address_size = sizeof(sockaddr_in);

    int peer_fd;
    while (true) {
        peer_fd = _accept(socket_fd, reinterpret_cast<sockaddr*>(&client_address), &client_address_size);

        if (peer_fd == -1) {
            *err = GetLastError();
            if (*err != nullptr && (*err)->GetErrorType() == ErrorType::kResourceTemporarilyUnavailable) {
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

hidra2::FileDescriptor hidra2::SystemIO::Open(const std::string& filename,
                                              int open_flags,
                                              Error* err) const {
    int flags = FileOpenModeToPosixFileOpenMode(open_flags);
    FileDescriptor fd = _open(filename.c_str(), flags);
    if(fd == -1) {
        *err = GetLastError();
        (*err)->Append(filename);
    } else {
        *err = nullptr;
    }
    return fd;
}

void hidra2::SystemIO::CloseSocket(SocketDescriptor fd, Error* err) const {
    if(err) {
        *err = nullptr;
    }
    if(!_close_socket(fd) && err) {
        *err = GetLastError();
    }
}

void hidra2::SystemIO::Close(FileDescriptor fd, Error* err) const {
    if(err) {
        *err = nullptr;
    }
    if(!_close(fd) && err) {
        *err = GetLastError();
    }
}

size_t hidra2::SystemIO::Read(FileDescriptor fd, void* buf, size_t length, Error* err) const {
    *err = nullptr;

    size_t already_read = 0;

    while(already_read < length) {
        ssize_t read_amount = _read(fd, (uint8_t*)buf + already_read, length - already_read);
        if(read_amount == 0) {
            *err = IOErrorTemplate::kEndOfFile.Copy();
            return already_read;
        }
        if (read_amount == -1) {
            *err = GetLastError();
            if (*err != nullptr) {
                return already_read;
            }
        }
        already_read += read_amount;
    }

    return already_read;
}

size_t hidra2::SystemIO::Write(FileDescriptor fd, const void* buf, size_t length, Error* err) const {
    *err = nullptr;

    size_t already_wrote = 0;

    while(already_wrote < length) {
        ssize_t write_amount = _write(fd, (uint8_t*)buf + already_wrote, length - already_wrote);
        if(write_amount == 0) {
            *err = IOErrorTemplate::kEndOfFile.Copy();
            return already_wrote;
        }
        if (write_amount == -1) {
            *err = GetLastError();
            if (*err != nullptr) {
                return already_wrote;
            }
        }
        already_wrote += write_amount;
    }

    return already_wrote;
}


short hidra2::SystemIO::AddressFamilyToPosixFamily(AddressFamilies address_family) const {
    switch (address_family) {
    case AddressFamilies::INET:
        return AF_INET;
    }
    return -1;
}

int hidra2::SystemIO::SocketTypeToPosixType(SocketTypes socket_type) const {
    switch (socket_type) {
    case SocketTypes::STREAM:
        return SOCK_STREAM;
    }
    return -1;
}

int hidra2::SystemIO::SocketProtocolToPosixProtocol(SocketProtocols socket_protocol) const {
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
    if(domain == -1) {
        *err = IOErrorTemplate::kUnsupportedAddressFamily.Copy();
        return -1;
    }

    int type = SocketTypeToPosixType(socket_type);
    if(type == -1) {
        *err = IOErrorTemplate::kUnknownError.Copy();
        return -1;
    }

    int protocol = SocketProtocolToPosixProtocol(socket_protocol);
    if(protocol == -1) {
        *err = IOErrorTemplate::kUnknownError.Copy();
        return -1;
    }

    SocketDescriptor socket_fd = _socket(domain, type, protocol);
    if(socket_fd == -1) {
        *err = GetLastError();
        return socket_fd;
    }

    *err = nullptr;

    ApplyNetworkOptions(socket_fd, err);

    return socket_fd;
}

void hidra2::SystemIO::InetBind(SocketDescriptor socket_fd, const std::string& address,
                                Error* err) const {
    *err = nullptr;

    int family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if (family == -1) {
        *err = IOErrorTemplate::kUnsupportedAddressFamily.Copy();
        return;
    }

    auto host_port_tuple = SplitAddressToHostnameAndPort(address);
    if (!host_port_tuple) {
        *err = IOErrorTemplate::kInvalidAddressFormat.Copy();
        return;
    }
    std::string host;
    uint16_t port = 0;
    std::tie(host, port) = *host_port_tuple;

    sockaddr_in socket_address{};
    socket_address.sin_addr.s_addr = inet_addr(host.c_str());
    socket_address.sin_port = htons(port);
    socket_address.sin_family = family;

    if (::bind(socket_fd, reinterpret_cast<const sockaddr*>(&socket_address), sizeof(socket_address)) == -1) {
        *err = GetLastError();
    }
}

void hidra2::SystemIO::Listen(SocketDescriptor socket_fd, int backlog, Error* err) const {
    *err = nullptr;

    if (_listen(socket_fd, backlog) == -1) {
        *err = GetLastError();
    }
}


size_t hidra2::SystemIO::ReceiveTimeout(SocketDescriptor socket_fd, void* buf, size_t length, long timeout_in_usec,
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
        *err = IOErrorTemplate::kTimeout.Copy();
        return 0;
    }
    if (res == -1) {
        *err = GetLastError();
        return 0;
    }

    return Receive(socket_fd, buf, length, err);
}


size_t hidra2::SystemIO::Receive(SocketDescriptor socket_fd, void* buf, size_t length, Error* err) const {

    size_t already_received = 0;

    while (already_received < length) {
        ssize_t received_amount = _recv(socket_fd, (uint8_t*) buf + already_received, length - already_received);
        if (received_amount == 0) {
            *err = IOErrorTemplate::kEndOfFile.Copy();
            return already_received;
        }
        if (received_amount == -1) {
            *err = GetLastError();
            if (*err != nullptr) {
                if((*err)->GetErrorType() == ErrorType::kResourceTemporarilyUnavailable) {
                    continue;
                }
                return already_received;
            }
        }
        already_received += received_amount;
    }
    *err = nullptr;
    return already_received;
}



size_t hidra2::SystemIO::Send(SocketDescriptor socket_fd,
                              const void* buf,
                              size_t length,
                              Error* err) const {

    size_t already_sent = 0;

    while (already_sent < length) {
        ssize_t send_amount = _send(socket_fd, (uint8_t*) buf + already_sent, length - already_sent);
        if (send_amount == 0) {
            *err = IOErrorTemplate::kEndOfFile.Copy();
            return already_sent;
        }
        if (send_amount == -1) {
            *err = GetLastError();
            if (*err != nullptr) {
                if((*err)->GetErrorType() == ErrorType::kResourceTemporarilyUnavailable) {
                    continue;
                }
                return already_sent;
            }
        }
        already_sent += send_amount;
    }

    *err = nullptr;

    return already_sent;
}



}
