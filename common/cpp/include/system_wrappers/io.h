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

    virtual FileData GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err) = 0;

    virtual int open(const char* __file, int __oflag) = 0;
    virtual int close(int __fd) = 0;
    virtual int64_t read(int __fd, void* buf, size_t count) = 0;
    virtual int64_t write(int __fd, const void* __buf, size_t __n) = 0;

// this is not standard function - to be implemented differently in windows and linux
    virtual std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) = 0;
};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__IO_H
