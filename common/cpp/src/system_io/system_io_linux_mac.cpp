

#include <cstring>

#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <zconf.h>
#include <netdb.h>

#include "system_io.h"

using std::string;
using std::vector;
using std::chrono::system_clock;

namespace asapo {

/**
 * \defgroup SYSTEM_IO_LINUX_PRIVATE
 * Local and private function that are being used by system_io_linux.cpp
 * @{
 */

Error GetLastErrorFromErrno() {
    switch (errno) {
    case 0:
        return nullptr;
    case EINTR:
        return TextError("Interrupt occurred, is a debugger attached?");
    case EBADF:
        return IOErrorTemplates::kBadFileNumber.Generate();
    case EAGAIN:
        return IOErrorTemplates::kResourceTemporarilyUnavailable.Generate();
    case ENETUNREACH:
        return IOErrorTemplates::kAddressNotValid.Generate();
    case ENOENT:
    case ENOTDIR:
        return IOErrorTemplates::kFileNotFound.Generate();
    case EACCES:
        return IOErrorTemplates::kPermissionDenied.Generate();
    case EFAULT:
        return IOErrorTemplates::kInvalidMemoryAddress.Generate();
    case EEXIST:
        return IOErrorTemplates::kFileAlreadyExists.Generate();
    case ENOSPC:
        return IOErrorTemplates::kNoSpaceLeft.Generate();
    case ECONNREFUSED:
        return IOErrorTemplates::kConnectionRefused.Generate();
    case EADDRINUSE:
        return IOErrorTemplates::kAddressAlreadyInUse.Generate();
    case ECONNRESET:
        return IOErrorTemplates::kConnectionResetByPeer.Generate();
    case ENOTSOCK:
        return IOErrorTemplates::kSocketOperationOnNonSocket.Generate();
    case ENOPROTOOPT:
        return IOErrorTemplates::kSocketOperationUnknownAtLevel.Generate();
    case EDOM:
        return IOErrorTemplates::kSocketOperationValueOutOfBound.Generate();
    case EPIPE:
        return IOErrorTemplates::kBrokenPipe.Generate();
    default:
        std::cout << "[IOErrorsFromErrno] Unknown error code: " << errno << std::endl;
        Error err = IOErrorTemplates::kUnknownIOError.Generate();
        (*err).Append("Unknown error code: " + std::to_string(errno));
        return err;
    }
};

Error SystemIO::GetLastError() const {
    return GetLastErrorFromErrno();
}

bool IsDirectory(const struct dirent* entity) {
    return entity->d_type == DT_DIR &&
           strstr(entity->d_name, "..") == nullptr &&
           strstr(entity->d_name, ".") == nullptr;
}

void SetModifyDate(const struct stat& t_stat, MessageMeta* message_meta) {
#ifdef __APPLE__
#define st_mtim st_mtimespec
#endif
    std::chrono::nanoseconds d = std::chrono::nanoseconds {t_stat.st_mtim.tv_nsec} +
                                 std::chrono::seconds{t_stat.st_mtim.tv_sec};
#ifdef __APPLE__
#undef st_mtim
#endif

    message_meta->timestamp = system_clock::time_point
    {std::chrono::duration_cast<system_clock::duration>(d)};
}

void SetFileSize(const struct stat& t_stat, MessageMeta* message_meta) {
    message_meta->size = t_stat.st_size;
}

void SetFileName(const string& name, MessageMeta* message_meta) {
    message_meta->name = name;
}

struct stat FileStat(const string& fname, Error* err) {
    struct stat t_stat {};
    errno = 0;
    int res = stat(fname.c_str(), &t_stat);
    if (res < 0) {
        *err = GetLastErrorFromErrno();
    }
    return t_stat;
}

MessageMeta GetMessageMeta(const string& name, Error* err) {
    MessageMeta message_meta;

    SetFileName(name, &message_meta);

    auto t_stat = FileStat(name, err);
    if (*err != nullptr) {
        (*err)->Append(name);
        return MessageMeta{};
    }

    SetFileSize(t_stat, &message_meta);

    SetModifyDate(t_stat, &message_meta);

    return message_meta;
}

MessageMeta SystemIO::GetMessageMeta(const string& name, Error* err) const {
    return ::asapo::GetMessageMeta(name, err);
}

void ProcessFileEntity(const struct dirent* entity, const std::string& path,
                       MessageMetas* files, Error* err) {

    *err = nullptr;
    if (entity->d_type != DT_REG) {
        return;
    }

    MessageMeta message_meta = GetMessageMeta(path + "/" + entity->d_name, err);
    if (*err != nullptr) {
        return;
    }
    files->push_back(message_meta);
}

void SystemIO::GetSubDirectoriesRecursively(const std::string& path, SubDirList* subdirs, Error* err) const {
    errno = 0;
    auto dir = opendir((path).c_str());
    if (dir == nullptr) {
        *err = GetLastError();
        (*err)->Append(path);
        return;
    }

    while (struct dirent* current_entity = readdir(dir)) {
        if (IsDirectory(current_entity)) {
            std::string subdir = path + "/" + current_entity->d_name;
            subdirs->push_back(subdir);
            GetSubDirectoriesRecursively(subdir, subdirs, err);
        }
        if (*err != nullptr) {
            errno = 0;
            closedir(dir);
            return;
        }
    }
    *err = GetLastError();
    closedir(dir);
}

void SystemIO::CollectMessageMetarmationRecursively(const std::string& path,
                                                 MessageMetas* files, Error* err) const {
    errno = 0;
    auto dir = opendir((path).c_str());
    if (dir == nullptr) {
        *err = GetLastError();
        (*err)->Append(path);
        return;
    }

    while (struct dirent* current_entity = readdir(dir)) {
        if (IsDirectory(current_entity)) {
            CollectMessageMetarmationRecursively(path + "/" + current_entity->d_name,
                                              files, err);
        } else {
            ProcessFileEntity(current_entity, path, files, err);
        }
        if (*err != nullptr) {
            errno = 0;
            closedir(dir);
            return;
        }
    }
    *err = GetLastError();
    closedir(dir);
}

void SystemIO::ApplyNetworkOptions(SocketDescriptor socket_fd, Error* err) const {
    //TODO: Need to change network layer code, so everything can be NonBlocking
    int flag = 1;
    if (
        /*(flags = fcntl(socket_fd, F_GETFL, 0)) == -1
        ||
        fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1
        ||*/
        setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (char*) &kNetBufferSize, sizeof(kNetBufferSize)) != 0
        ||
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) != 0 ||
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) != 0 ||
        setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) != 0

    ) {
        *err = GetLastError();
    }
}

FileDescriptor SystemIO::_open(const char* filename, int posix_open_flags) const {
    return ::open(filename, posix_open_flags, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
}

bool SystemIO::_close(asapo::FileDescriptor fd) const {
    return ::close(fd) == 0;
}

ssize_t SystemIO::_read(asapo::FileDescriptor fd, void* buffer, size_t length) {
    return ::read(fd, buffer, length);
}

ssize_t SystemIO::_write(asapo::FileDescriptor fd, const void* buffer, size_t length) {
    return ::write(fd, buffer, length);
}

SocketDescriptor SystemIO::_socket(int address_family, int socket_type, int socket_protocol) const {
    return ::socket(address_family, socket_type, socket_protocol);
}

ssize_t SystemIO::_send(SocketDescriptor socket_fd, const void* buffer, size_t length) {
    return ::send(socket_fd, buffer, length, MSG_NOSIGNAL);
}

ssize_t SystemIO::_recv(SocketDescriptor socket_fd, void* buffer, size_t length) {
    return ::recv(socket_fd, buffer, length, 0);
}

int SystemIO::_mkdir(const char* dirname) const {
    return ::mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

int SystemIO::_listen(SocketDescriptor socket_fd, int backlog) const {
    return ::listen(socket_fd, backlog);
}

SocketDescriptor SystemIO::_connect(SocketDescriptor socket_fd, const void* address, size_t address_length) const {
    return ::connect(socket_fd, static_cast<const sockaddr*>(address), static_cast<socklen_t>(address_length));
}

SocketDescriptor SystemIO::_accept(SocketDescriptor socket_fd, void* address, size_t* address_length) const {
    return ::accept(socket_fd, static_cast<sockaddr*>(address), reinterpret_cast<socklen_t*>(address_length));
}

bool SystemIO::_close_socket(SocketDescriptor socket_fd) const {
    return ::close(socket_fd) == 0;
}

std::string SystemIO::AddressFromSocket(SocketDescriptor socket) const noexcept {
    socklen_t len;
    struct sockaddr_storage addr;
    char ipstr[INET6_ADDRSTRLEN];
    int port;

    len = sizeof addr;
    auto res = getpeername(socket, (struct sockaddr*) &addr, &len);
    if (res != 0) {
        return GetLastError()->Explain();
    }
    if (addr.ss_family == AF_INET) {
        struct sockaddr_in* s = (struct sockaddr_in*) &addr;
        port = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
    } else { // AF_INET6
        struct sockaddr_in6* s = (struct sockaddr_in6*) &addr;
        port = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
    }

    return std::string(ipstr) + ':' + std::to_string(port);
}

}
