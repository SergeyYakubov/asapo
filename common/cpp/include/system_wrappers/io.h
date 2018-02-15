#ifndef HIDRA2_SYSTEM_WRAPPERS__IO_H
#define HIDRA2_SYSTEM_WRAPPERS__IO_H

#include <cinttypes>

#include <string>
#include <vector>
#include <chrono>

#include "common/data_structs.h"
#include "common/error.h"

namespace hidra2 {

namespace IOErrors {
auto const kFileNotFound = "No such file or directory";
auto const kReadError = "Read error";
auto const kPermissionDenied = "Permission denied";
auto const kUnknownError = "Unknown error";
auto const kMemoryAllocationError = "Memory Allocation Error";
}

Error IOErrorFromErrno();


class IO {
  public:

    virtual FileData GetDataFromFile(const std::string& fname, uint64_t fsize, Error* err) const noexcept = 0;
    virtual uint64_t Read(int fd, uint8_t* array, uint64_t fsize, Error* err) const noexcept = 0;
    virtual std::string ReadFileToString(const std::string& fname, Error* err)const noexcept = 0;
    virtual int open(const char* __file, int __oflag) const noexcept = 0;
    virtual int close(int __fd) const noexcept = 0;

// this is not standard function - to be implemented differently in windows and linux
    virtual FileInfos FilesInFolder(const std::string& folder, Error* err) const = 0;
};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__IO_H
