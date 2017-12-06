#include <gmock/gmock.h>

#include "worker/data_broker.h"
#include "system_wrappers/io.h"
#include "system_wrappers/system_io.h"
#include "../src/folder_data_broker.h"



using hidra2::DataBrokerFactory;
using hidra2::DataBroker;
using hidra2::FolderDataBroker;
using hidra2::WorkerErrorCode;
using hidra2::IO;
using hidra2::IOErrors;
using hidra2::FileInfo;

using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;

namespace {

TEST(FolderDataBroker, SetCorrectIO) {
    auto data_broker = new FolderDataBroker("test");
    ASSERT_THAT(dynamic_cast<hidra2::SystemIO*>(data_broker->io__.release()), Ne(nullptr));
}



class FakeIO: public IO {
  public:
    int open(const char* __file, int __oflag) {
        return 0;
    };
    int close(int __fd) {
        return 0;
    };
    ssize_t read(int __fd, void* buf, size_t count) {
        return 0;
    };
    ssize_t write(int __fd, const void* __buf, size_t __n) {
        return 0;
    };
    std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) {
        *err = IOErrors::NO_ERROR;
        return {};
    }
};

class IOFolderNotFound: public FakeIO {
  public:
    std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) {
        *err = IOErrors::FOLDER_NOT_FOUND;
        return {};
    }
};

class IOFodlerUnknownError: public FakeIO {
  public:
    std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) {
        *err = IOErrors::UNKWOWN_ERROR;
        return {};
    }
};


class FolderDataBrokerTests : public Test {
  public:
    std::unique_ptr<FolderDataBroker> data_broker;
    void SetUp() override {
        data_broker = std::unique_ptr<FolderDataBroker> {new FolderDataBroker("/path/to/file")};
        data_broker->io__ = std::unique_ptr<IO> {new FakeIO()};
    }
    void TearDown() override {
    }
};

TEST_F(FolderDataBrokerTests, CanConnect) {
    auto return_code = data_broker->Connect();

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::ERR__NO_ERROR));
}

TEST_F(FolderDataBrokerTests, CannotConnectWhenNoFolder) {
    data_broker->io__ = std::unique_ptr<IO> {new IOFolderNotFound()};

    auto return_code = data_broker->Connect();

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::SOURCE_NOT_FOUND));
}

TEST_F(FolderDataBrokerTests, ConnectReturnsUnknownIOError) {
    data_broker->io__ = std::unique_ptr<IO> {new IOFodlerUnknownError()};

    auto return_code = data_broker->Connect();

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::UNKNOWN_IO_ERROR));
}



}
