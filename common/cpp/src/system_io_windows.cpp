#include "system_wrappers/system_io.h"

#include <cstring>
#include <sys/stat.h>
#include <algorithm>
#include <io.h>
#include <windows.h>

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

std::chrono::system_clock::time_point FileTime2TimePoint(const FILETIME& ft,IOErrors* err)
{
    SYSTEMTIME st = {0};
    if (!FileTimeToSystemTime(&ft, &st)) {
        *err = IOErrorFromGetLastError();
        return {};
    }
    // number of seconds
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;

    time_t secs = ull.QuadPart / 10000000ULL - 11644473600ULL;
    std::chrono::milliseconds ms((ull.QuadPart / 10000ULL) % 1000);

    auto tp = std::chrono::system_clock::from_time_t(secs);
    tp += ms;
    *err=IOErrors::kNoError;
    return tp;
}

bool IsDirectory(const WIN32_FIND_DATA f) {
    return (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        strstr(f.cFileName, "..") == nullptr &&
        strstr(f.cFileName, ".") == nullptr;
}

void ProcessFileEntity(const WIN32_FIND_DATA f, const std::string &path,
                       std::vector<FileInfo> &files, IOErrors *err) {

    *err = IOErrors::kNoError;
    if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return;
    }

    FileInfo file_info;
    file_info.modify_date=FileTime2TimePoint(f.ftLastWriteTime,err);
    if (*err != IOErrors::kNoError) {
        return;
    }

    file_info.base_name=f.cFileName;
    file_info.relative_path=path;
    files.push_back(file_info);
}


void SystemIO::CollectFileInformationRecursivly(const std::string &path,
                                      std::vector<FileInfo> &files, IOErrors *err) {
    WIN32_FIND_DATA find_data;
    HANDLE handle = FindFirstFile((path+"\\*.*").c_str(), &find_data);
    if (handle == INVALID_HANDLE_VALUE) {
        *err = IOErrorFromGetLastError();
        return;
    }

    do
    {
        if (IsDirectory(find_data))
        {
            CollectFileInformationRecursivly(path + "\\" + find_data.cFileName,files,err);
        } else {
            ProcessFileEntity(find_data,path, files, err);
        }
        if (*err != IOErrors::kNoError) {
            FindClose(handle);
            return;
        }
    } while (FindNextFile(handle, &find_data));

    if (FindClose(handle)){
        *err=IOErrors ::kNoError;
    }else{
        *err = IOErrorFromGetLastError();
    }

}

int64_t SystemIO::read(int __fd, void *buf, size_t count) {
    return (int64_t) _read(__fd, buf, (unsigned int) count);
}

int64_t SystemIO::write(int __fd, const void *__buf, size_t __n) {
    return (int64_t) _write(__fd, __buf, (unsigned int) __n);
}

int SystemIO::open(const char* __file, int __oflag) {
    int fd;
    errno = _sopen_s(&fd,__file,__oflag,_SH_DENYNO,_S_IREAD | _S_IWRITE);
    return fd;
}

int SystemIO::close(int __fd) {
    return ::_close(__fd);
}


}
