#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H

#include "io.h"

namespace hidra2 {

class SystemIO final : public IO {
  public:
    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, IOError* err) const noexcept override;
    int open(const char* __file, int __oflag) const noexcept override;
    int close(int __fd) const noexcept override;
    int64_t read(int __fd, void* buf, size_t count) const noexcept override;
    int64_t write(int __fd, const void* __buf, size_t __n) const noexcept override;
    FileInfos FilesInFolder(const std::string& folder, IOError* err) const override;
  private:
    void ReadWholeFile(int fd, uint8_t* array, uint64_t fsize, IOError* err) const noexcept;
    void CollectFileInformationRecursivly(const std::string& path,
                                          FileInfos* files, IOError* err) const;

};
}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
