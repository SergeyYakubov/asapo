#ifndef HIDRA2_COMMON__MOCKIO_H
#define HIDRA2_COMMON__MOCKIO_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "system_wrappers/io.h"

namespace hidra2 {

class MockIO : public IO {
  public:
    std::string ReadFileToString(const std::string& fname, Error* err)const noexcept  override {
        SimpleError* error;
        auto data = ReadFileToString_t(fname,  &error);
        err->reset(error);
        return data;
    }

    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, Error* err) const noexcept override {
        SimpleError* error;
        auto data = GetDataFromFile_t(fname, fsize, &error);
        err->reset(error);
        return FileData(data);
    }
    int open(const char* __file, int __oflag) const noexcept override {
        return 0;
    }
    int close(int __fd) const noexcept override {
        return 0;
    }
    uint64_t Read(int fd, uint8_t* array, uint64_t fsize, Error* err) const noexcept override {
        return 0;
    }

    FileInfos FilesInFolder(const std::string& folder, Error* err) const override {
        SimpleError* error;
        auto data = FilesInFolder_t(folder, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD2(ReadFileToString_t,
                       std::string (const std::string& fname, SimpleError** err));

    MOCK_CONST_METHOD3(GetDataFromFile_t,
                       uint8_t* (const std::string& fname, uint64_t fsize, SimpleError** err));
    MOCK_CONST_METHOD2(FilesInFolder_t,
                       FileInfos(
                           const std::string& folder, hidra2::SimpleError
                           ** err));

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