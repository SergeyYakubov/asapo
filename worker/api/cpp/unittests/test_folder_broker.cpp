#include <gmock/gmock.h>
#include "gtest/gtest.h"
using ::testing::AtLeast;

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
using hidra2::FileData;


using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;
using ::testing::_;
using ::testing::Mock;


namespace {

TEST(FolderDataBroker, SetCorrectIO) {
    auto data_broker = new FolderDataBroker("test");
    ASSERT_THAT(dynamic_cast<hidra2::SystemIO*>(data_broker->io__.get()), Ne(nullptr));
    delete data_broker;
}



class FakeIO: public IO {
  public:
    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err)override {
        *err = IOErrors::NO_ERROR;
        return {};
    };

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
        std::vector<FileInfo> file_infos;
        FileInfo fi;
        fi.size = 100;
        fi.base_name = "1";
        file_infos.push_back(fi);
        fi.base_name = "2";
        file_infos.push_back(fi);
        fi.base_name = "3";
        file_infos.push_back(fi);

        return file_infos;
    }
};

class IOFolderNotFound: public FakeIO {
  public:
    std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) {
        *err = IOErrors::FILE_NOT_FOUND;
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

class IOEmptyFodler: public FakeIO {
  public:
    std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) {
        *err = IOErrors::NO_ERROR;
        return {};
    }
};

class IOCannotOpenFile: public FakeIO {
  public:
    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err)  {
        *err = IOErrors::PERMISSIONS_DENIED;
        return {};
    };
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

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::OK));
}

TEST_F(FolderDataBrokerTests, CannotConnectTwice) {
    data_broker->Connect();

    auto return_code = data_broker->Connect();

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::SOURCE_ALREADY_CONNECTED));
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

TEST_F(FolderDataBrokerTests, GetNextWithoutConnectReturnsError) {
    auto err = data_broker->GetNext(nullptr, nullptr);

    ASSERT_THAT(err, Eq(WorkerErrorCode::SOURCE_NOT_CONNECTED));
}

TEST_F(FolderDataBrokerTests, GetNextWithNullPointersReturnsError) {
    data_broker->Connect();

    auto err = data_broker->GetNext(nullptr, nullptr);

    ASSERT_THAT(err, Eq(WorkerErrorCode::WRONG_INPUT));
}


TEST_F(FolderDataBrokerTests, GetNextReturnsFileInfo) {
    data_broker->Connect();
    FileInfo fi;

    auto err = data_broker->GetNext(&fi, nullptr);

    ASSERT_THAT(err, Eq(WorkerErrorCode::OK));
    ASSERT_THAT(fi.base_name, Eq("1"));
    ASSERT_THAT(fi.size, Eq(100));

}

TEST_F(FolderDataBrokerTests, SecondNextReturnsAnotherFileInfo) {
    data_broker->Connect();
    FileInfo fi;
    data_broker->GetNext(&fi, nullptr);

    auto err = data_broker->GetNext(&fi, nullptr);

    ASSERT_THAT(err, Eq(WorkerErrorCode::OK));
    ASSERT_THAT(fi.base_name, Eq("2"));
}

TEST_F(FolderDataBrokerTests, GetNextFromEmptyFolderReturnsError) {
    data_broker->io__ = std::unique_ptr<IO> {new IOEmptyFodler()};
    data_broker->Connect();
    FileInfo fi;

    auto err = data_broker->GetNext(&fi, nullptr);
    ASSERT_THAT(err, Eq(WorkerErrorCode::NO_DATA));
}


TEST_F(FolderDataBrokerTests, GetNextReturnsErrorWhenFilePermissionsDenied) {
    data_broker->io__ = std::unique_ptr<IO> {new IOCannotOpenFile()};
    data_broker->Connect();
    FileInfo fi;
    FileData data;

    auto err = data_broker->GetNext(&fi, &data);
    ASSERT_THAT(err, Eq(WorkerErrorCode::PERMISSIONS_DENIED));
}


class OpenFileMock : public FakeIO {
  public:
    MOCK_METHOD3(GetDataFromFile, FileData(const std::string&, uint64_t, IOErrors*));
};


class GetDataFromFileTests : public Test {
  public:
    std::unique_ptr<FolderDataBroker> data_broker;
    OpenFileMock mock;
    FileInfo fi;
    FileData data;
    void SetUp() override {
        data_broker = std::unique_ptr<FolderDataBroker> {new FolderDataBroker("/path/to/file")};
        data_broker->io__ = std::unique_ptr<IO> {&mock};
        data_broker->Connect();
    }
    void TearDown() override {
        data_broker->io__.release();
    }
};

TEST_F(GetDataFromFileTests, GetNextCallsGetDataFileWithFileName) {
    EXPECT_CALL(mock, GetDataFromFile("/path/to/file/1", _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(IOErrors::NO_ERROR), testing::Return(FileData{})));

    data_broker->GetNext(&fi, &data);
}


TEST_F(GetDataFromFileTests, GetNextReturnsDataAndInfo) {
    EXPECT_CALL(mock, GetDataFromFile(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(IOErrors::NO_ERROR), testing::Return(FileData{'1'})));

    data_broker->GetNext(&fi, &data);

    ASSERT_THAT(data[0], Eq('1'));
    ASSERT_THAT(fi.base_name, Eq("1"));

}

TEST_F(GetDataFromFileTests, GetNextReturnsOnlyData) {
    EXPECT_CALL(mock, GetDataFromFile(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(IOErrors::NO_ERROR), testing::Return(FileData{'1'})));

    data_broker->GetNext(nullptr, &data);

    ASSERT_THAT(data[0], Eq('1'));
}


TEST_F(GetDataFromFileTests, GetNextReturnsErrorWhenCannotReadData) {
    EXPECT_CALL(mock, GetDataFromFile(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(IOErrors::READ_ERROR), testing::Return(FileData{})));

    auto err = data_broker->GetNext(&fi, &data);

    ASSERT_THAT(err, Eq(WorkerErrorCode::ERROR_READING_FROM_SOURCE));
    ASSERT_TRUE(data.empty());
}


}
