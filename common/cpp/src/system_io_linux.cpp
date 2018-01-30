#include "system_wrappers/system_io.h"

#include <cstring>


#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <zconf.h>

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
    case EFAULT:
        return IOErrors::kInvalidMemoryAddress;
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

SocketDescriptor SystemIO::_socket(int address_family, int socket_type, int socket_protocol) const {
    return ::socket(address_family, socket_type, socket_protocol);
}

ssize_t SystemIO::_send(SocketDescriptor socket_fd, const void* buffer, size_t length) const {
    return ::send(socket_fd, buffer, length, 0);
}

ssize_t SystemIO::_recv(SocketDescriptor socket_fd, void* buffer, size_t length) const {
    return ::recv(socket_fd, buffer, length, 0);
}

int SystemIO::_mkdir(const char* dirname) const {
    return ::mkdir(dirname, S_IRWXU);
}

int SystemIO::_listen(SocketDescriptor socket_fd, int backlog) const {
    return ::listen(socket_fd, backlog);
}


void SystemIO::_close_socket(SocketDescriptor socket_fd) const {
    ::close(socket_fd);
}

}
