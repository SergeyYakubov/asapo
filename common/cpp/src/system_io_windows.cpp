#include "system_wrappers/system_io.h"

#include <cstring>
#include <sys/stat.h>
#include <algorithm>
#include <io.h>
#include <direct.h>
#include <iostream>

using std::string;
using std::vector;
using std::chrono::system_clock;

namespace hidra2 {

IOErrors IOErrorFromGetLastError() {
    DWORD last_error = GetLastError();
    switch (last_error) {
    case ERROR_SUCCESS:
        return IOErrors::kNoError;
    case ERROR_PATH_NOT_FOUND:
    case ERROR_FILE_NOT_FOUND:
        return IOErrors::kFileNotFound;
    case ERROR_ACCESS_DENIED:
        return IOErrors::kPermissionDenied;
    case ERROR_CONNECTION_REFUSED:
        return IOErrors::kConnectionRefused;
    case WSAEFAULT:
        return IOErrors::kInvalidMemoryAddress;
    case WSAECONNRESET:
        return IOErrors::kConnectionResetByPeer;
    case WSAENOTSOCK:
        return IOErrors::kSocketOperationOnNonSocket;
    default:
        std::cout << "[IOErrorFromGetLastError] Unknown error code: " << last_error << std::endl;
        return IOErrors::kUnknownError;
    }
}


IOErrors SystemIO::GetLastError() const {
    return IOErrorFromGetLastError();
}

IOErrors CheckFileTime(const FILETIME& ft) {
    SYSTEMTIME st = { 0 };
    if (!FileTimeToSystemTime(&ft, &st)) {
        return IOErrorFromGetLastError();
    }
    return IOErrors::kNoError;
}

constexpr auto kShift = 11644473600ULL;
constexpr auto k100nsInSec = 10000000ULL;

uint64_t GetLinuxEpochSecFromWindowsEpoch(ULARGE_INTEGER ull) {
    //    ull.QuadPart is amount of 100ns intervals since Windows Epoch
    return (uint64_t)ull.QuadPart / k100nsInSec - kShift;
}

uint64_t GetLinuxNanosecFromWindowsEpoch(ULARGE_INTEGER ull) {
    return (uint64_t)(ull.QuadPart % k100nsInSec) * 100;
}

std::chrono::system_clock::time_point FileTime2TimePoint(const FILETIME& ft, IOErrors* err) {

    *err = CheckFileTime(ft);
    if (*err != IOErrors::kNoError) {
        return std::chrono::system_clock::time_point{};
    }

    // number of seconds
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;

    auto sec = GetLinuxEpochSecFromWindowsEpoch(ull);
    auto nsec = GetLinuxNanosecFromWindowsEpoch(ull);

    std::chrono::nanoseconds d = std::chrono::nanoseconds{ nsec } +
                                 std::chrono::seconds{ sec };

    auto tp = system_clock::time_point
    { std::chrono::duration_cast<std::chrono::system_clock::duration>(d) };

    *err = IOErrors::kNoError;
    return tp;
}

bool IsDirectory(const WIN32_FIND_DATA f) {
    return (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
           strstr(f.cFileName, "..") == nullptr &&
           strstr(f.cFileName, ".") == nullptr;
}

void ProcessFileEntity(const WIN32_FIND_DATA f, const std::string& path,
                       std::vector<FileInfo>* files, IOErrors* err) {

    *err = IOErrors::kNoError;
    if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return;
    }

    FileInfo file_info;
    file_info.modify_date = FileTime2TimePoint(f.ftLastWriteTime, err);
    if (*err != IOErrors::kNoError) {
        return;
    }

    file_info.base_name = f.cFileName;
    file_info.relative_path = path;
    files->push_back(file_info);
}


void SystemIO::CollectFileInformationRecursivly(const std::string& path,
                                                std::vector<FileInfo>* files, IOErrors* err) const {
    WIN32_FIND_DATA find_data;
    HANDLE handle = FindFirstFile((path + "\\*.*").c_str(), &find_data);
    if (handle == INVALID_HANDLE_VALUE) {
        *err = IOErrorFromGetLastError();
        return;
    }

    do {
        if (IsDirectory(find_data)) {
            CollectFileInformationRecursivly(path + "\\" + find_data.cFileName, files, err);
        } else {
            ProcessFileEntity(find_data, path, files, err);
        }
        if (*err != IOErrors::kNoError) {
            FindClose(handle);
            return;
        }
    } while (FindNextFile(handle, &find_data));

    if (FindClose(handle)) {
        *err = IOErrors::kNoError;
    } else {
        *err = IOErrorFromGetLastError();
    }
}

FileDescriptor SystemIO::_open(const char* filename, int posix_open_flags) const {
    int fd;
    errno = _sopen_s(&fd, filename, posix_open_flags, _SH_DENYNO, _S_IREAD | _S_IWRITE);
    return fd;
}

bool SystemIO::_close(FileDescriptor fd) const {
	return ::_close(fd) == 0;
}

bool SystemIO::_close_socket(SocketDescriptor fd) const {
	return ::closesocket(fd) == 0;
}

ssize_t SystemIO::_read(FileDescriptor fd, void* buffer, size_t length) const {
    return ::_read(fd, (char*)buffer, length);
}

ssize_t SystemIO::_write(FileDescriptor fd, const void* buffer, size_t length) const {
    return ::_write(fd, (const char*)buffer, length);
}

SocketDescriptor SystemIO::_socket(int address_family, int socket_type, int socket_protocol) const {
    static bool WSAStartupDone = false;
    if (!WSAStartupDone) {
        WSAStartupDone = true;
        WORD wVersionRequested = MAKEWORD(2, 2);
        WSADATA wsaData;
        int err = WSAStartup(wVersionRequested, &wsaData);
        if (err != 0) {
            std::cout << "[_socket/WSAStartup] Faild to WSAStartup with version 2.2" << std::endl;
            WSACleanup();
            // Do not return, since ::socket has to set an errno
        } else {
            std::atexit([] {
                WSACleanup();
            });
        }
    }

    return ::socket(address_family, socket_type, socket_protocol);
}

SocketDescriptor SystemIO::_connect(SocketDescriptor socket_fd, const void* address, size_t address_length) const {
    return ::connect(socket_fd, static_cast<const sockaddr*>(address), address_length);
}

ssize_t SystemIO::_send(SocketDescriptor socket_fd, const void* buffer, size_t length) const {
    return ::send(socket_fd, (char*)buffer, length, 0);
}

ssize_t SystemIO::_recv(SocketDescriptor socket_fd, void* buffer, size_t length) const {
    return ::recv(socket_fd, (char*)buffer, length, 0);
}

int SystemIO::_mkdir(const char* dirname) const {
    return ::_mkdir(dirname);
}

int SystemIO::_listen(SocketDescriptor fd, int backlog) const {
    return ::listen(fd, backlog);
}

SocketDescriptor SystemIO::_accept(SocketDescriptor socket_fd, void* address, size_t* address_length) const {
    return ::accept(socket_fd, static_cast<sockaddr*>(address), (int*)address_length);
}

}