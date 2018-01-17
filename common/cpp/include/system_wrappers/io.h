#ifndef HIDRA2_SYSTEM_WRAPPERS__IO_H
#define HIDRA2_SYSTEM_WRAPPERS__IO_H

#include <cinttypes>

#include <string>
#include <vector>
#include <chrono>

#include "common/file_info.h"

namespace hidra2 {

enum class IOErrors {
    kNoError,
    kFileNotFound,
    kReadError,
    kPermissionDenied,
    kUnknownError,
    kMemoryAllocationError
};

IOErrors IOErrorFromErrno();


class IO {
  public:

    virtual FileData GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err) const noexcept = 0;

    virtual int open(const char* __file, int __oflag) const noexcept = 0;
    virtual int close(int __fd) const noexcept = 0;
    virtual int64_t read(int __fd, void* buf, size_t count) const noexcept = 0;
    virtual int64_t write(int __fd, const void* __buf, size_t __n) const noexcept = 0;

// this is not standard function - to be implemented differently in windows and linux
    virtual FileInfos FilesInFolder(const std::string& folder, IOErrors* err) const = 0;
};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__IO_H
