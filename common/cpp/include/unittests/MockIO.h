#ifndef HIDRA2_COMMON__MOCKIO_H
#define HIDRA2_COMMON__MOCKIO_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "system_wrappers/io.h"

namespace hidra2 {

class MockIO : public IO {
  public:
    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, IOError* err) const noexcept override {
        return GetDataFromFile_t(fname, fsize, err);
    }
    int open(const char* __file, int __oflag) const noexcept override {
        return open_t(__file, __oflag);
    }
    int close(int __fd) const noexcept override {
        return close_t(__fd);
    }
    int64_t read(int __fd, void* buf, size_t count) const noexcept override {
        return read_t(__fd, buf, count);
    }
    int64_t write(int __fd, const void* __buf, size_t __n) const noexcept override {
        return write_t(__fd, __buf, __n);
    }
    MOCK_CONST_METHOD3(GetDataFromFile_t,
                       FileData(const std::string& fname, uint64_t fsize, IOError* err));
    MOCK_CONST_METHOD2(FilesInFolder,
                       FileInfos(
                           const std::string& folder, IOError
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