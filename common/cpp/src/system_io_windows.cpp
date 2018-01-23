#include "system_wrappers/system_io.h"

#include <cstring>
#include <sys/stat.h>
#include <algorithm>
#include <io.h>
#include <windows.h>
#include <direct.h>

using std::string;
using std::vector;
using std::chrono::system_clock;

namespace hidra2 {

IOErrors IOErrorFromGetLastError() {
    IOErrors err;
    switch (GetLastError()) {
    case ERROR_SUCCESS :
        err = IOErrors::kNoError;
        break;
    case ERROR_PATH_NOT_FOUND:
    case ERROR_FILE_NOT_FOUND:
        err = IOErrors::kFileNotFound;
        break;
    case ERROR_ACCESS_DENIED:
        err = IOErrors::kPermissionDenied;
        break;
    default:
        err = IOErrors::kUnknownError;
        break;
    }
    return err;
}


IOErrors SystemIO::GetLastError() const {
    return IOErrorFromGetLastError();
}

IOErrors CheckFileTime(const FILETIME& ft) {
    SYSTEMTIME st = {0};
    if (!FileTimeToSystemTime(&ft, &st)) {
        return IOErrorFromGetLastError();
    }
    return IOErrors::kNoError;
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

    std::chrono::nanoseconds d = std::chrono::nanoseconds {nsec} +
                                 std::chrono::seconds{sec};

    auto tp = system_clock::time_point
    {std::chrono::duration_cast<std::chrono::system_clock::duration>(d)};

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
        *err = IOErrors ::kNoError;
    } else {
        *err = IOErrorFromGetLastError();
    }
}


void SystemIO::_close(hidra2::FileDescriptor fd) const {
    ::_close(fd);
}

ssize_t SystemIO::_read(hidra2::FileDescriptor fd, void* buffer, size_t length) const {
    return ::_read(fd, buffer, length);
}

ssize_t SystemIO::_write(hidra2::FileDescriptor fd, const void* buffer, size_t length) const {
    return ::_write(fd, buffer, length);
}

FileDescriptor SystemIO::_socket(int address_family, int socket_type, int socket_protocol) const {
    return ::socket(address_family, socket_type, socket_protocol);
}

ssize_t SystemIO::_send(FileDescriptor socket_fd, const void* buffer, size_t length) const {
    return ::send(socket_fd, (char*)buffer, length, 0);
}

ssize_t SystemIO::_recv(FileDescriptor socket_fd, void* buffer, size_t length) const {
    return ::recv(socket_fd, (char*)buffer, length, 0);
}

int SystemIO::_mkdir(const char* dirname) const {
    return ::_mkdir(dirname);
}

int SystemIO::_listen(FileDescriptor fd, int backlog) const {
    return ::listen(fd, backlog);
}



}

