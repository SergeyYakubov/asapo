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

/**
 * \defgroup SYSTEM_IO_LINUX_PRIVATE
 * Local and private function that are being used by system_io_linux.cpp
 * @{
 */

IOErrors GetLastErrorFromErrno() {
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
    case ENOTSOCK:
        return IOErrors::kSocketOperationOnNonSocket;
    default:
        std::cout << "[IOErrorsFromErrno] Unknown error code: " << errno << std::endl;
        return IOErrors::kUnknownError;
    }
};

IOErrors SystemIO::GetLastError() const {
    return GetLastErrorFromErrno();
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
        *err = GetLastErrorFromErrno();
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
                       std::vector<FileInfo>* files, IOErrors* err) {

    *err = IOErrors::kNoError;
    if (entity->d_type != DT_REG) {
        return;
    }

    FileInfo file_info = GetFileInfo(path, entity->d_name, err);
    if (*err != IOErrors::kNoError) {
        return;
    }

    files->push_back(file_info);
}


/** @} */

void SystemIO::CollectFileInformationRecursivly(const std::string& path,
                                                std::vector<FileInfo>* files,
                                                IOErrors* err) const {
    auto dir = ::opendir((path).c_str());
    if (dir == nullptr) {
        *err = GetLastError();
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
    *err = GetLastError();
    closedir(dir);
}


int SystemIO::AddressFamilyToPosixFamily(AddressFamilies address_family) const {
    switch (address_family) {
    case AddressFamilies::INET:
        return AF_INET;
    }
    return -1;
};

int SystemIO::SocketTypeToPosixType(SocketTypes socket_type) const {
    switch (socket_type) {
    case SocketTypes::STREAM:
        return SOCK_STREAM;
    }
    return -1;
}

int SystemIO::SocketProtocolToPosixProtocol(SocketProtocols socket_protocol) const {
    switch (socket_protocol) {
    case SocketProtocols::IP:
        return IPPROTO_IP;
    }
    return -1;
}

hidra2::FileDescriptor hidra2::SystemIO::_open(const char* filename, int posix_open_flags) const {
    return ::open(filename, posix_open_flags, S_IWUSR | S_IRWXU);
}

void SystemIO::_close(hidra2::FileDescriptor fd) const {
    ::close(fd);
}

ssize_t SystemIO::_read(hidra2::FileDescriptor fd, void* buffer, size_t length) const {
    return ::read(fd, buffer, length);
}

ssize_t SystemIO::_write(hidra2::FileDescriptor fd, const void* buffer, size_t length) const {
    return ::write(fd, buffer, length);
}

FileDescriptor SystemIO::_socket(int address_family, int socket_type, int socket_protocol) const {
    return ::socket(address_family, socket_type, socket_protocol);
}

ssize_t SystemIO::_send(FileDescriptor socket_fd, const void* buffer, size_t length) const {
    return ::send(socket_fd, buffer, length, 0);
}

ssize_t SystemIO::_recv(FileDescriptor socket_fd, void* buffer, size_t length) const {
    return ::recv(socket_fd, buffer, length, 0);
}

int SystemIO::_mkdir(const char* dirname) const {
    return ::mkdir(dirname, S_IRWXU);
}

int SystemIO::_listen(FileDescriptor fd, int backlog) const {
    return ::listen(fd, backlog);
}


void hidra2::SystemIO::InetBind(hidra2::FileDescriptor socket_fd,
                                const std::string& address,
                                uint16_t port,
                                hidra2::IOErrors* err) const {
    *err = IOErrors::kNoError;

    int family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if (family == -1) {
        *err = IOErrors::kUnsupportedAddressFamily;
        return;
    }

    sockaddr_in socket_address{};
    socket_address.sin_addr.s_addr = inet_addr(address.c_str());
    socket_address.sin_port = htons(port);
    socket_address.sin_family = static_cast<sa_family_t>(family);

    if (::bind(socket_fd, reinterpret_cast<const sockaddr*>(&socket_address), sizeof(socket_address)) == -1) {
        *err = GetLastError();
    }

}

void hidra2::SystemIO::InetConnect(FileDescriptor socket_fd, const std::string& address, hidra2::IOErrors* err) const {
    *err = IOErrors::kNoError;
    std::string host;
    uint16_t port = 0;

    try {
        host = address.substr(0, address.find(':'));

        std::string port_str = address.substr(address.find(':') + 1, address.length());
        port = static_cast<uint16_t>(std::stoi(port_str));
    } catch (std::exception& e) {
        *err = IOErrors::kInvalidAddressFormat;
        return;
    }

    sa_family_t family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if (family == -1) {
        *err = IOErrors::kUnsupportedAddressFamily;
        return;
    }

    sockaddr_in socket_address{};
    socket_address.sin_addr.s_addr = inet_addr(host.c_str());
    socket_address.sin_port = htons(port);
    socket_address.sin_family = family;

    if (::connect(socket_fd, (struct sockaddr*) &socket_address, sizeof(socket_address)) == -1) {
        *err = GetLastError();
        return;
    }
}


std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>> hidra2::SystemIO::InetAccept(
hidra2::FileDescriptor socket_fd, IOErrors* err) const {
    *err = IOErrors::kNoError;
    sa_family_t family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if (family == -1) {
        *err = IOErrors::kUnsupportedAddressFamily;
        return nullptr;
    }

    sockaddr_in client_address{};
    socklen_t client_address_size = sizeof(sockaddr_in);

    int peer_fd = ::accept(socket_fd, reinterpret_cast<sockaddr*>(&client_address), &client_address_size);

    if (peer_fd == -1) {
        *err = GetLastError();
        return nullptr;
    }

    std::string
    address = std::string(inet_ntoa(client_address.sin_addr)) + ':' + std::to_string(client_address.sin_port);
    return std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>>(new
            std::tuple<std::string,
            hidra2::FileDescriptor>(
                address,
                peer_fd));
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
    if (res == 0) {
        *err = IOErrors::kTimeout;
        return 0;
    }
    if (res == -1) {
        *err = GetLastError();
        return 0;
    }

    return Receive(socket_fd, buf, length, err);
}

}
