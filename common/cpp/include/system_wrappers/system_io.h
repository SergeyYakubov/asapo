#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H

#include "io.h"

namespace hidra2 {

class SystemIO final : public IO {
  public:
    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err) override;
    int open(const char* __file, int __oflag) override;
    int close(int __fd) override;
    ssize_t read(int __fd, void* buf, size_t count) override;
    ssize_t write(int __fd, const void* __buf, size_t __n) override;
    std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) override;
};
}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
