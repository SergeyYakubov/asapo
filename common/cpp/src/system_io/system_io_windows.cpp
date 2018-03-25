#include "system_wrappers/system_io.h"

#include <cstring>
#include <sys/stat.h>
#include <algorithm>
#include <io.h>
#include <windows.h>
#include <fcntl.h>
#include <iostream>
#include <direct.h>


using std::string;
using std::vector;
using std::chrono::system_clock;

namespace hidra2 {

Error IOErrorFromGetLastError() {
    DWORD last_error = GetLastError();
    switch (last_error) {
    case ERROR_SUCCESS:
        return nullptr;
    case ERROR_PATH_NOT_FOUND:
    case ERROR_FILE_NOT_FOUND:
        return IOErrorTemplates::kFileNotFound.Generate();
    case ERROR_ACCESS_DENIED:
        return IOErrorTemplates::kPermissionDenied.Generate();
    case ERROR_CONNECTION_REFUSED:
        return IOErrorTemplates::kConnectionRefused.Generate();
    case WSAEFAULT:
        return IOErrorTemplates::kInvalidMemoryAddress.Generate();
    case WSAECONNRESET:
        return IOErrorTemplates::kConnectionResetByPeer.Generate();
    case WSAENOTSOCK:
        return IOErrorTemplates::kSocketOperationOnNonSocket.Generate();
    case WSAEWOULDBLOCK:
        return IOErrorTemplates::kResourceTemporarilyUnavailable.Generate();
    case WSAECONNREFUSED:
        return IOErrorTemplates::kConnectionRefused.Generate();
    default:
        std::cout << "[IOErrorFromGetLastError] Unknown error code: " << last_error << std::endl;
        Error err = IOErrorTemplates::kUnknownIOError.Generate();
        (*err).Append("Unknown error code: " + std::to_string(errno));
        return err;
    }
}

Error SystemIO::GetLastError() const {
    return IOErrorFromGetLastError();
}

Error CheckFileTime(const FILETIME& ft) {
    SYSTEMTIME st = { 0 };
    if (!FileTimeToSystemTime(&ft, &st)) {
        return IOErrorFromGetLastError();
    }
    return nullptr;
}

constexpr auto kShift = 11644473600ULL;
constexpr auto k100nsInSec = 10000000ULL;

uint64_t GetLinuxEpochSecFromWindowsEpoch(ULARGE_INTEGER ull) {
//    ull.QuadPart is amount of 100ns intervals since Windows Epoch
    return (uint64_t) ull.QuadPart / k100nsInSec - kShift;
}

uint64_t GetLinuxNanosecFromWindowsEpoch(ULARGE_INTEGER ull) {
    return (uint64_t)(ull.QuadPart % k100nsInSec) * 100;
}

std::chrono::system_clock::time_point FileTime2TimePoint(const FILETIME& ft, Error* err) {

    *err = CheckFileTime(ft);
    if (*err) {
        return std::chrono::system_clock::time_point{};
    }

    // number of seconds
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;

    auto sec = GetLinuxEpochSecFromWindowsEpoch(ull);
    auto nsec = GetLinuxNanosecFromWindowsEpoch(ull);

    std::chrono::nanoseconds d = std::chrono::nanoseconds {nsec} +
                                 std::chrono::seconds{sec};

    auto tp = system_clock::time_point
    {std::chrono::duration_cast<std::chrono::system_clock::duration>(d)};

    *err = nullptr;
    return tp;
}

bool IsDirectory(const WIN32_FIND_DATA f) {
    return (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
           strstr(f.cFileName, "..") == nullptr &&
           strstr(f.cFileName, ".") == nullptr;
}

FileInfo GetFileInfo_win(const WIN32_FIND_DATA& f, const string& name, Error* err) {
    FileInfo file_info;

    file_info.modify_date = FileTime2TimePoint(f.ftLastWriteTime, err);
    if (*err) {
        return {};
    }

    ULARGE_INTEGER fsize;
    fsize.LowPart = f.nFileSizeLow;
    fsize.HighPart = f.nFileSizeHigh;

    file_info.size = fsize.QuadPart;

    file_info.name = name + "\\" + f.cFileName;

    return file_info;
}


FileInfo SystemIO::GetFileInfo(const std::string& name, Error* err) const {
    WIN32_FIND_DATA f;

    auto hFind = FindFirstFile(name.c_str(), &f);
    if (hFind == INVALID_HANDLE_VALUE) {
        *err = IOErrorFromGetLastError();
        (*err)->Append(name);
        return {};
    }
    FindClose(hFind);
    return GetFileInfo_win(f, name, err);
}

void ProcessFileEntity(const WIN32_FIND_DATA& f, const std::string& path,
                       FileInfos* files, Error* err) {

    *err = nullptr;
    if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return;
    }

    auto file_info = GetFileInfo_win(f, path, err);
    if (*err) {
        return;
    }

    files->push_back(file_info);
}

void SystemIO::CollectFileInformationRecursively(const std::string& path,
                                                 FileInfos* files, Error* err) const {
    WIN32_FIND_DATA find_data;
    HANDLE handle = FindFirstFile((path + "\\*.*").c_str(), &find_data);
    if (handle == INVALID_HANDLE_VALUE) {
        *err = IOErrorFromGetLastError();
        (*err)->Append(path);
        return;
    }

    do {
        if (IsDirectory(find_data)) {
            CollectFileInformationRecursively(path + "\\" + find_data.cFileName, files, err);
        } else {
            ProcessFileEntity(find_data, path, files, err);
        }
        if (*err) {
            FindClose(handle);
            return;
        }
    } while (FindNextFile(handle, &find_data));

    if (FindClose(handle)) {
        *err = nullptr;
    } else {
        *err = IOErrorFromGetLastError();
    }

}

void hidra2::SystemIO::ApplyNetworkOptions(SocketDescriptor socket_fd, Error* err) const {
    //TODO: Seeing issues when using these settings - need further investigation
    //Event if NonBlockingIO is set, it seems that _recv is a blocking call :/
    /*
    static u_long iMode = 1;

    if (
    	ioctlsocket(socket_fd, FIONBIO, &iMode) != 0
    	||
    	setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (char*)&kNetBufferSize, sizeof(kNetBufferSize)) != 0
    	||
    	setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (char*)&kNetBufferSize, sizeof(kNetBufferSize)) != 0
    	) {
    	*err = GetLastError();
    }
    */
}

FileDescriptor SystemIO::_open(const char* filename, int posix_open_flags) const {
    int fd;
    errno = _sopen_s(&fd, filename, posix_open_flags | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
    return fd;
}

bool SystemIO::_close(FileDescriptor fd) const {
    return ::_close(fd) == 0;
}

bool SystemIO::_close_socket(SocketDescriptor fd) const {
    return ::closesocket(fd) == 0;
}

SocketDescriptor SystemIO::_socket(int address_family, int socket_type, int socket_protocol) const {
    InitializeSocketIfNecessary();
    return ::socket(address_family, socket_type, socket_protocol);
}

SocketDescriptor SystemIO::_connect(SocketDescriptor socket_fd, const void* address, size_t address_length) const {
    return ::connect(socket_fd, static_cast<const sockaddr*>(address), address_length);
}

ssize_t SystemIO::_read(FileDescriptor fd, void* buffer, size_t length) {
    return ::_read(fd, (char*)buffer, length);
}

ssize_t SystemIO::_write(FileDescriptor fd, const void* buffer, size_t length) {
    return ::_write(fd, (const char*)buffer, length);
}

ssize_t SystemIO::_send(SocketDescriptor socket_fd, const void* buffer, size_t length) {
    return ::send(socket_fd, (char*)buffer, length, 0);
}

ssize_t SystemIO::_recv(SocketDescriptor socket_fd, void* buffer, size_t length) {
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

void SystemIO::InitializeSocketIfNecessary() const {
    static bool WSAStartupDone = false;
    if (!WSAStartupDone) {
        WSAStartupDone = true;
        WORD wVersionRequested = MAKEWORD(2, 2);
        WSADATA wsaData;
        int err = WSAStartup(wVersionRequested, &wsaData);
        if (err != 0) {
            std::cout << "[_socket/WSAStartup] Failed to WSAStartup with version 2.2" << std::endl;
            WSACleanup();
            // Do not return, since ::socket has to set an errno
        } else {
            std::atexit([] {
                WSACleanup();
            });
        }
    }
}

}
