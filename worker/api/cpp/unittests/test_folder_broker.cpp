#include <gmock/gmock.h>
#include "gtest/gtest.h"

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

using ::testing::AtLeast;
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

    virtual uint8_t* GetDataFromFileProxy(const std::string& fname, uint64_t fsize, IOErrors* err) const {
        *err = IOErrors::kNoError;
        return nullptr;
    };

    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err) const noexcept override {
        return FileData(GetDataFromFileProxy(fname, fsize, err));
    };

    int open(const char* __file, int __oflag) const noexcept override {
        return 0;
    };

    int close(int __fd)const noexcept override {
        return 0;
    };

    uint64_t Read(int fd, uint8_t* array, uint64_t fsize, IOErrors* err) const noexcept override {
        return 0;
    }

    std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) const override {
        *err = IOErrors::kNoError;
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
    std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) const override {
        *err = IOErrors::kFileNotFound;
        return {};
    }
};

class IOFodlerUnknownError: public FakeIO {
  public:
    std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) const override {
        *err = IOErrors::kUnknownError;
        return {};
    }
};

class IOEmptyFodler: public FakeIO {
  public:
    std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) const override {
        *err = IOErrors::kNoError;
        return {};
    }
};

class IOCannotOpenFile: public FakeIO {
  public:
    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err) const noexcept override {
        *err = IOErrors::kPermissionDenied;
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

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::kOK));
}

TEST_F(FolderDataBrokerTests, CannotConnectTwice) {
    data_broker->Connect();

    auto return_code = data_broker->Connect();

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::kSourceAlreadyConnected));
}


TEST_F(FolderDataBrokerTests, CannotConnectWhenNoFolder) {
    data_broker->io__ = std::unique_ptr<IO> {new IOFolderNotFound()};

    auto return_code = data_broker->Connect();

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::kSourceNotFound));
}

TEST_F(FolderDataBrokerTests, ConnectReturnsUnknownIOError) {
    data_broker->io__ = std::unique_ptr<IO> {new IOFodlerUnknownError()};

    auto return_code = data_broker->Connect();

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::kUnknownIOError));
}

TEST_F(FolderDataBrokerTests, GetNextWithoutConnectReturnsError) {
    auto err = data_broker->GetNext(nullptr, nullptr);

    ASSERT_THAT(err, Eq(WorkerErrorCode::kSourceNotConnected));
}

TEST_F(FolderDataBrokerTests, GetNextWithNullPointersReturnsError) {
    data_broker->Connect();

    auto err = data_broker->GetNext(nullptr, nullptr);

    ASSERT_THAT(err, Eq(WorkerErrorCode::kWrongInput));
}

TEST_F(FolderDataBrokerTests, GetNextReturnsFileInfo) {
    data_broker->Connect();
    FileInfo fi;

    auto err = data_broker->GetNext(&fi, nullptr);

    ASSERT_THAT(err, Eq(WorkerErrorCode::kOK));
    ASSERT_THAT(fi.base_name, Eq("1"));
    ASSERT_THAT(fi.size, Eq(100));

}

TEST_F(FolderDataBrokerTests, SecondNextReturnsAnotherFileInfo) {
    data_broker->Connect();
    FileInfo fi;
    data_broker->GetNext(&fi, nullptr);

    auto err = data_broker->GetNext(&fi, nullptr);

    ASSERT_THAT(err, Eq(WorkerErrorCode::kOK));
    ASSERT_THAT(fi.base_name, Eq("2"));
}

TEST_F(FolderDataBrokerTests, GetNextFromEmptyFolderReturnsError) {
    data_broker->io__ = std::unique_ptr<IO> {new IOEmptyFodler()};
    data_broker->Connect();
    FileInfo fi;

    auto err = data_broker->GetNext(&fi, nullptr);
    ASSERT_THAT(err, Eq(WorkerErrorCode::kNoData));
}


TEST_F(FolderDataBrokerTests, GetNextReturnsErrorWhenFilePermissionsDenied) {
    data_broker->io__ = std::unique_ptr<IO> {new IOCannotOpenFile()};
    data_broker->Connect();
    FileInfo fi;
    FileData data;

    auto err = data_broker->GetNext(&fi, &data);
    ASSERT_THAT(err, Eq(WorkerErrorCode::kPermissionDenied));
}


class OpenFileMock : public FakeIO {
  public:
    MOCK_CONST_METHOD3(GetDataFromFileProxy, uint8_t* (const std::string&, uint64_t, IOErrors*));
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
    EXPECT_CALL(mock, GetDataFromFileProxy("/path/to/file/1", _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(IOErrors::kNoError), testing::Return(nullptr)));

    data_broker->GetNext(&fi, &data);
}



TEST_F(GetDataFromFileTests, GetNextReturnsDataAndInfo) {
    EXPECT_CALL(mock, GetDataFromFileProxy(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(IOErrors::kNoError), testing::Return(new uint8_t[1] {'1'})));

    data_broker->GetNext(&fi, &data);

    ASSERT_THAT(data[0], Eq('1'));
    ASSERT_THAT(fi.base_name, Eq("1"));

}

TEST_F(GetDataFromFileTests, GetNextReturnsOnlyData) {
    EXPECT_CALL(mock, GetDataFromFileProxy(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(IOErrors::kNoError), testing::Return(new uint8_t[1] {'1'})));

    data_broker->GetNext(nullptr, &data);

    ASSERT_THAT(data[0], Eq('1'));
}


TEST_F(GetDataFromFileTests, GetNextReturnsErrorWhenCannotReadData) {
    EXPECT_CALL(mock, GetDataFromFileProxy(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(IOErrors::kReadError), testing::Return(nullptr)));

    auto err = data_broker->GetNext(&fi, &data);

    ASSERT_THAT(err, Eq(WorkerErrorCode::kErrorReadingSource));
}

TEST_F(GetDataFromFileTests, GetNextReturnsErrorWhenCannotAllocateData) {
    EXPECT_CALL(mock, GetDataFromFileProxy(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(IOErrors::kMemoryAllocationError), testing::Return(nullptr)));

    auto err = data_broker->GetNext(&fi, &data);

    ASSERT_THAT(err, Eq(WorkerErrorCode::kMemoryError));
}


}
