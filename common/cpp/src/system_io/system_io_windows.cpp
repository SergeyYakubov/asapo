#include "system_wrappers/system_io.h"

#include <cstring>
#include <sys/stat.h>
#include <algorithm>
#include <io.h>
#include <windows.h>

#include <fcntl.h>


using std::string;
using std::vector;
using std::chrono::system_clock;

namespace hidra2 {

Error IOErrorFromGetLastError() {
    const char* message;
    switch (GetLastError()) {
    case ERROR_SUCCESS :
        return nullptr;
    case ERROR_PATH_NOT_FOUND:
    case ERROR_FILE_NOT_FOUND:
        message = IOErrors::kFileNotFound;
        break;
    case ERROR_ACCESS_DENIED:
        message = IOErrors::kPermissionDenied;
        break;
    default:
        message = IOErrors::kUnknownError;
        break;
    }
    return TextError(message);
}

Error CheckFileTime(const FILETIME& ft) {
    SYSTEMTIME st = {0};
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

FileInfo GetFileInfo_win(const WIN32_FIND_DATA& f, const string& name, Error* err)  {
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


FileInfo GetFileInfo(const string& name, Error* err) {
    WIN32_FIND_DATA f;

    auto hFind = FindFirstFile(name.c_str() , &f);
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


void SystemIO::CollectFileInformationRecursivly(const std::string& path,
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
            CollectFileInformationRecursivly(path + "\\" + find_data.cFileName, files, err);
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

int64_t SystemIO::read(int __fd, void* buf, size_t count) const noexcept {
    return (int64_t) _read(__fd, buf, (unsigned int) count);
}

int64_t SystemIO::write(int __fd, const void* __buf, size_t __n) const noexcept {
    return (int64_t) _write(__fd, __buf, (unsigned int) __n);
}

int SystemIO::open(const char* __file, int __oflag) const noexcept {
    int fd;
    errno = _sopen_s(&fd, __file, __oflag | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
    return fd;
}

int SystemIO::close(int __fd) const noexcept {
    return ::_close(__fd);
}


}
