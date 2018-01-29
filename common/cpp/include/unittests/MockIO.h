#ifndef HIDRA2_COMMON__MOCKIO_H
#define HIDRA2_COMMON__MOCKIO_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "system_wrappers/io.h"

namespace hidra2 {

class MockIO : public IO {
  public:
    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err) const noexcept override {
        return FileData{GetDataFromFile_t(fname, fsize, err)};
    }
    int open(const char* __file, int __oflag) const noexcept override {
        return 0;
    }
    int close(int __fd) const noexcept override {
        return 0;
    }
    uint64_t Read(int fd, uint8_t* array, uint64_t fsize, IOErrors* err) const noexcept override {
        return 0;
    }

    MOCK_CONST_METHOD3(GetDataFromFile_t,
                       uint8_t* (const std::string& fname, uint64_t fsize, IOErrors* err));
    MOCK_CONST_METHOD2(FilesInFolder,
                       FileInfos(
                           const std::string& folder, IOErrors
                           *err));

    MOCK_CONST_METHOD3(read_t,
                       int64_t(int
                               __fd, void* buf, size_t
                               count));

    MOCK_CONST_METHOD3(write_t,
                       int64_t(int
                               __fd,
                               const void* __buf, size_t
                               __n));

    MOCK_CONST_METHOD2(open_t,
                       int(const char* __file, int __oflag));

    MOCK_CONST_METHOD1(close_t,
                       int(int __fd));
};

}

#endif //HIDRA2_COMMON__MOCKIO_H