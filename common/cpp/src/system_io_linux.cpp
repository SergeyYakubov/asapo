#include "system_wrappers/system_io.h"

#include <cstring>

#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <zconf.h>
#include <assert.h>

using std::string;
using std::vector;
using std::chrono::system_clock;

namespace hidra2 {

IOErrors IOErrorsFromErrno() {
    switch (errno) {
    case 0:
        return IOErrors::kNoError;
    case EBADF:
        return IOErrors::kBadFileNumber;
    case ENOENT:
    case ENOTDIR:
        return IOErrors::kFileNotFound;
    case EACCES:
        return IOErrors::kPermissionDenied;
    case EEXIST:
        return IOErrors::kFileAlreadyExists;
    case ENOSPC:
        return IOErrors::kNoSpaceLeft;
    case ECONNREFUSED:
        return IOErrors::kConnectionRefused;
    case EADDRINUSE:
        return IOErrors::kAddressAlreadyInUse;
    case ECONNRESET:
        return IOErrors::kConnectionResetByPeer;
    default:
        std::cout << "[IOErrorsFromErrno] Unknown error code: " << errno << std::endl;
        return IOErrors::kUnknownError;
    }
}

sa_family_t AddressFamilyToPosixFamily(AddressFamilies address_family) {
    switch(address_family) {
    case AddressFamilies::INET:
        return AF_INET;
    }
    return -1;
};

int FileOpenModeToPosixFileOpenMode(int open_flags) {
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

bool IsDirectory(const struct dirent* entity) {
    return entity->d_type == DT_DIR &&
           strstr(entity->d_name, "..") == nullptr &&
           strstr(entity->d_name, ".") == nullptr;
}

void SetModifyDate(const struct stat& t_stat, FileInfo* file_info) {
#ifdef __APPLE__
#define st_mtim st_mtimespec
#endif
    std::chrono::nanoseconds d = std::chrono::nanoseconds {t_stat.st_mtim.tv_nsec} +
                                 std::chrono::seconds{t_stat.st_mtim.tv_sec};
#ifdef __APPLE__
#undef st_mtim
#endif

    file_info->modify_date = system_clock::time_point
    {std::chrono::duration_cast<system_clock::duration>(d)};
}

void SetFileSize(const struct stat& t_stat, FileInfo* file_info) {
    file_info->size = t_stat.st_size;
}

void SetFileName(const string& path, const string& name, FileInfo* file_info) {
    file_info->relative_path = path;
    file_info->base_name = name;
}

struct stat FileStat(const string& fname, IOErrors* err) {
    struct stat t_stat {};
    int res = stat(fname.c_str(), &t_stat);
    if (res < 0) {
        *err = IOErrorsFromErrno();
    }
    return t_stat;
}

FileInfo GetFileInfo(const string& path, const string& name, IOErrors* err) {
    FileInfo file_info;

    SetFileName(path, name, &file_info);

    auto t_stat = FileStat(path + "/" + name, err);
    if (*err != IOErrors::kNoError) {
        return FileInfo{};
    }

    SetFileSize(t_stat, &file_info);

    SetModifyDate(t_stat, &file_info);

    return file_info;
}

void ProcessFileEntity(const struct dirent* entity, const std::string& path,
                       std::vector<FileInfo>& files, IOErrors* err) {

    *err = IOErrors::kNoError;
    if (entity->d_type != DT_REG) {
        return;
    }

    FileInfo file_info = GetFileInfo(path, entity->d_name, err);
    if (*err != IOErrors::kNoError) {
        return;
    }

    files.push_back(file_info);
}

void CollectFileInformationRecursivly(const std::string& path,
                                      std::vector<FileInfo>& files, IOErrors* err) {
    auto dir = opendir((path).c_str());
    if (dir == nullptr) {
        *err = IOErrorsFromErrno();
        return;
    }

    while (struct dirent* current_entity = readdir(dir)) {
        if (IsDirectory(current_entity)) {
            CollectFileInformationRecursivly(path + "/" + current_entity->d_name,
                                             files, err);
        } else {
            ProcessFileEntity(current_entity, path, files, err);
        }
        if (*err != IOErrors::kNoError) {
            closedir(dir);
            return;
        }
    }
    *err = IOErrorsFromErrno();
    closedir(dir);
}

hidra2::FileDescriptor hidra2::SystemIO::CreateSocket(hidra2::AddressFamilies address_family,
        hidra2::SocketTypes socket_type,
        hidra2::SocketProtocols socket_protocol,
        hidra2::IOErrors* err) const {
    *err = IOErrors::kNoError;

    int domain = AddressFamilyToPosixFamily(address_family);
    if(domain == -1) {
        *err = IOErrors::kUnsupportedAddressFamily;
        return -1;
    }

    int type = 0;
    switch(socket_type) {
    case SocketTypes::STREAM:
        type = SOCK_STREAM;
        break;
    default:
        *err = IOErrors::kUnknownError;//TODO
        return -1;
    }

    int protocol = 0;
    switch(socket_protocol) {
    case SocketProtocols::IP:
        protocol = IPPROTO_IP;
        break;
    default:
        *err = IOErrors::kUnknownError;//TODO
        return -1;
    }

    int fd = ::socket(domain, type, protocol);

    if(fd == -1) {
        *err = IOErrorsFromErrno();
        return -1;
    }

    return fd;
}

}

void hidra2::SystemIO::InetBind(hidra2::FileDescriptor socket_fd,
                                const std::string& address,
                                uint16_t port,
                                hidra2::IOErrors* err) const {
    *err = IOErrors::kNoError;

    sa_family_t family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if(family == -1) {
        *err = IOErrors::kUnsupportedAddressFamily;
        return;
    }

    sockaddr_in socket_address {};
    socket_address.sin_addr.s_addr  = inet_addr(address.c_str());
    socket_address.sin_port         = htons(port);
    socket_address.sin_family       = family;

    if(::bind(socket_fd, reinterpret_cast<const sockaddr*>(&socket_address), sizeof(socket_address)) == -1) {
        *err = IOErrorsFromErrno();
    }

}

void hidra2::SystemIO::Listen(hidra2::FileDescriptor socket_fd, int backlog, hidra2::IOErrors* err) const {
    *err = IOErrors::kNoError;

    if(::listen(socket_fd, backlog) == -1) {
        *err = IOErrorsFromErrno();
    }
}

std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>> hidra2::SystemIO::InetAccept(
hidra2::FileDescriptor socket_fd, IOErrors* err) const {
    *err = IOErrors::kNoError;
    sa_family_t family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if(family == -1) {
        *err = IOErrors::kUnsupportedAddressFamily;
        return nullptr;
    }

    sockaddr_in client_address {};
    socklen_t client_address_size = sizeof(sockaddr_in);

    int peer_fd = ::accept(socket_fd, reinterpret_cast<sockaddr*>(&client_address), &client_address_size);

    if(peer_fd == -1) {
        *err = IOErrorsFromErrno();
        return nullptr;
    }

    std::string address = std::string(inet_ntoa(client_address.sin_addr)) + ':' + std::to_string(client_address.sin_port);
    return std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>>(new
            std::tuple<std::string, hidra2::FileDescriptor>(address, peer_fd));
}

void hidra2::SystemIO::InetConnect(FileDescriptor socket_fd, const std::string& address, hidra2::IOErrors* err) const {
    *err = IOErrors::kNoError;
    std::string host;
    uint16_t port = 0;

    try {
        host = address.substr(0, address.find(':'));

        std::string port_str = address.substr(address.find(':') + 1, address.length());
        port = static_cast<uint16_t>(std::stoi(port_str));
    } catch(std::exception& e) {
        *err = IOErrors::kInvalidAddressFormat;
        return;
    }

    sa_family_t family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if(family == -1) {
        *err = IOErrors::kUnsupportedAddressFamily;
        return;
    }

    sockaddr_in socket_address {};
    socket_address.sin_addr.s_addr = inet_addr(host.c_str());
    socket_address.sin_port = htons(port);
    socket_address.sin_family = family;

    if(::connect(socket_fd, (struct sockaddr*)&socket_address, sizeof(socket_address)) == -1) {
        *err = IOErrorsFromErrno();
        return;
    }
}

size_t hidra2::SystemIO::Receive(hidra2::FileDescriptor socket_fd, void* buf, size_t length,
                                 hidra2::IOErrors* err) const {
    *err = hidra2::IOErrors::kNoError;

    size_t already_received = 0;

    while(already_received < length) {
        ssize_t received_amount = ::recv(socket_fd, (uint8_t*)buf + already_received, length - already_received, 0);
        if(received_amount == 0) {
            *err = IOErrors::kEndOfFile;
            return already_received;
        }
        if(received_amount == -1) {
            *err = IOErrorsFromErrno();
            return already_received;
        }
        already_received += received_amount;
    }

    return already_received;
}

size_t hidra2::SystemIO::ReceiveTimeout(hidra2::FileDescriptor socket_fd,
                                        void* buf,
                                        size_t length,
                                        uint16_t timeout_in_sec,
                                        hidra2::IOErrors* err) const {
    *err = hidra2::IOErrors::kNoError;

    fd_set read_fds;
    FD_SET(socket_fd, &read_fds);
    timeval timeout;
    timeout.tv_sec = timeout_in_sec;
    timeout.tv_usec = 0;

    int res = ::select(socket_fd + 1, &read_fds, nullptr, nullptr, &timeout);
    if(res == 0) {
        *err = IOErrors::kTimeout;
        return 0;
    }
    if(res == -1) {
        *err = IOErrorsFromErrno();
        return 0;
    }

    return Receive(socket_fd, buf, length, err);
}

size_t hidra2::SystemIO::Send(hidra2::FileDescriptor socket_fd,
                              const void* buf,
                              size_t length,
                              hidra2::IOErrors* err) const {
    *err = hidra2::IOErrors::kNoError;

    size_t already_sent = 0;

    while(already_sent < length) {
        ssize_t send_amount = ::send(socket_fd, (uint8_t*)buf + already_sent, length - already_sent, 0);
        if(send_amount == 0) {
            *err = IOErrors::kEndOfFile;
            return already_sent;
        }
        if(send_amount == -1) {
            *err = IOErrorsFromErrno();
            return already_sent;
        }
        already_sent += send_amount;
    }

    return already_sent;
}

hidra2::FileDescriptor hidra2::SystemIO::Open(const std::string& filename,
                                              int open_flags,
                                              IOErrors* err) const {
    *err = IOErrors::kNoError;
    int flags = FileOpenModeToPosixFileOpenMode(open_flags);

    FileDescriptor fd = ::open(filename.c_str(), flags, S_IWUSR | S_IRWXU);
    if(fd == -1) {
        *err = IOErrorsFromErrno();
    }

    return fd;
}

void hidra2::SystemIO::Close(hidra2::FileDescriptor fd, hidra2::IOErrors* err) const {
    int status = ::close(fd);
    if(!err) {
        return;
    }
    *err = IOErrors::kNoError;
    if(status == -1) {
        *err = IOErrorsFromErrno();
    }
}
size_t hidra2::SystemIO::Write(FileDescriptor fd, const void* buf, size_t length, IOErrors* err) const {
    *err = hidra2::IOErrors::kNoError;

    size_t already_sent = 0;

    while(already_sent < length) {
        ssize_t send_amount = ::write(fd, (uint8_t*)buf + already_sent, length - already_sent);
        if(send_amount == 0) {
            *err = IOErrors::kEndOfFile;
            return already_sent;
        }
        if(send_amount == -1) {
            *err = IOErrorsFromErrno();
            return already_sent;
        }
        already_sent += send_amount;
    }

    return already_sent;
}
void hidra2::SystemIO::CreateDirectory(const std::string& directory_name, hidra2::IOErrors* err) const {
    *err = IOErrors::kNoError;
    struct stat st = {0};
    int result = ::stat(directory_name.c_str(), &st);
    if (result != -1) {
        *err = IOErrors::kFileAlreadyExists;
        return;
    }
    if(result == -1) {
        mkdir(directory_name.c_str(), S_IRWXU);
        *err = IOErrorsFromErrno();
        return;
    }

}
hidra2::FileData hidra2::SystemIO::GetDataFromFile(const std::string& fname,
        uint64_t fsize,
        hidra2::IOErrors* err) const {
    return hidra2::FileData();
}

vector<hidra2::FileInfo> hidra2::SystemIO::FilesInFolder(const std::string& folder, hidra2::IOErrors* err) const {
    return vector<hidra2::FileInfo>();
}
