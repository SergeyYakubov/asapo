#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H

#include "io.h"

namespace hidra2 {

class SystemIO final : public IO {
  public:
    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, Error* err) const noexcept override;
    int open(const char* __file, int __oflag) const noexcept override;
    int close(int __fd) const noexcept override;
    uint64_t Read(int fd, uint8_t* array, uint64_t fsize, Error* err) const noexcept override;
    std::vector<FileInfo> FilesInFolder(const std::string& folder, Error* err) const override;
  private:
    void CollectFileInformationRecursivly(const std::string& path,
                                          std::vector<FileInfo>* files, Error* err) const;
    int64_t read(int __fd, void* buf, size_t count) const noexcept;
    int64_t write(int __fd, const void* __buf, size_t __n) const noexcept;
};
}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
