#include <gmock/gmock.h>
#include "gtest/gtest.h"

#include "worker/data_broker.h"
#include "system_wrappers/io.h"
#include "system_wrappers/system_io.h"
#include "../src/folder_data_broker.h"

using hidra2::DataBrokerFactory;
using hidra2::DataBroker;
using hidra2::FolderDataBroker;
using hidra2::IO;
using hidra2::FileInfos;
using hidra2::FileInfo;
using hidra2::FileData;
using hidra2::Error;
using hidra2::TextError;
using hidra2::SimpleError;

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

    virtual uint8_t* GetDataFromFileProxy(const std::string& fname, uint64_t fsize, SimpleError** err) const {
        *err = nullptr;
        return nullptr;
    };

    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, Error* err) const noexcept override {
        SimpleError* error;
        auto data = GetDataFromFileProxy(fname, fsize, &error);
        err->reset(error);
        return FileData(data);
    };

    int open(const char* __file, int __oflag) const noexcept override {
        return 0;
    };

    int close(int __fd)const noexcept override {
        return 0;
    };

    uint64_t Read(int fd, uint8_t* array, uint64_t fsize, Error* err) const noexcept override {
        return 0;
    };
    FileInfos FilesInFolder(const std::string& folder, Error* err) const override {
        *err = nullptr;
        FileInfos file_infos;
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
    FileInfos FilesInFolder(const std::string& folder, Error* err) const override {
        *err = hidra2::TextError(hidra2::IOErrors::kFileNotFound);
        return {};
    }
};

class IOFolderUnknownError: public FakeIO {
  public:
    FileInfos FilesInFolder(const std::string& folder, Error* err) const override {
        *err = hidra2::TextError(hidra2::IOErrors::kUnknownError);
        return {};
    }
};

class IOEmptyFolder: public FakeIO {
  public:
    FileInfos FilesInFolder(const std::string& folder, Error* err) const override {
        *err = nullptr;
        return {};
    }
};

class IOCannotOpenFile: public FakeIO {
  public:
    FileData GetDataFromFile(const std::string& fname, uint64_t fsize, Error* err) const noexcept override {
        *err = hidra2::TextError(hidra2::IOErrors::kPermissionDenied);
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

    ASSERT_THAT(return_code, Eq(nullptr));
}

TEST_F(FolderDataBrokerTests, CannotConnectTwice) {
    data_broker->Connect();

    auto return_code = data_broker->Connect();

    ASSERT_THAT(return_code->Explain(), Eq(hidra2::WorkerErrorMessage::kSourceAlreadyConnected));
}


TEST_F(FolderDataBrokerTests, CannotConnectWhenNoFolder) {
    data_broker->io__ = std::unique_ptr<IO> {new IOFolderNotFound()};

    auto return_code = data_broker->Connect();

    ASSERT_THAT(return_code->Explain(), Eq(hidra2::IOErrors::kFileNotFound));
}

TEST_F(FolderDataBrokerTests, ConnectReturnsUnknownIOError) {
    data_broker->io__ = std::unique_ptr<IO> {new IOFolderUnknownError()};

    auto return_code = data_broker->Connect();

    ASSERT_THAT(return_code->Explain(), Eq(hidra2::IOErrors::kUnknownError));
}

TEST_F(FolderDataBrokerTests, GetNextWithoutConnectReturnsError) {
    auto err = data_broker->GetNext(nullptr, nullptr);

    ASSERT_THAT(err->Explain(), Eq(hidra2::WorkerErrorMessage::kSourceNotConnected));
}

TEST_F(FolderDataBrokerTests, GetNextWithNullPointersReturnsError) {
    data_broker->Connect();

    auto err = data_broker->GetNext(nullptr, nullptr);

    ASSERT_THAT(err->Explain(), Eq(hidra2::WorkerErrorMessage::kWrongInput));
}

TEST_F(FolderDataBrokerTests, GetNextReturnsFileInfo) {
    data_broker->Connect();
    FileInfo fi;

    auto err = data_broker->GetNext(&fi, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(fi.base_name, Eq("1"));
    ASSERT_THAT(fi.size, Eq(100));

}

TEST_F(FolderDataBrokerTests, SecondNextReturnsAnotherFileInfo) {
    data_broker->Connect();
    FileInfo fi;
    data_broker->GetNext(&fi, nullptr);

    auto err = data_broker->GetNext(&fi, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(fi.base_name, Eq("2"));
}

TEST_F(FolderDataBrokerTests, GetNextFromEmptyFolderReturnsError) {
    data_broker->io__ = std::unique_ptr<IO> {new IOEmptyFolder()};
    data_broker->Connect();
    FileInfo fi;

    auto err = data_broker->GetNext(&fi, nullptr);
    ASSERT_THAT(err->Explain(), Eq(hidra2::WorkerErrorMessage::kNoData));
}


TEST_F(FolderDataBrokerTests, GetNextReturnsErrorWhenFilePermissionsDenied) {
    data_broker->io__ = std::unique_ptr<IO> {new IOCannotOpenFile()};
    data_broker->Connect();
    FileInfo fi;
    FileData data;

    auto err = data_broker->GetNext(&fi, &data);
    ASSERT_THAT(err->Explain(), Eq(hidra2::IOErrors::kPermissionDenied));
}


class OpenFileMock : public FakeIO {
  public:
    MOCK_CONST_METHOD3(GetDataFromFileProxy, uint8_t* (const std::string&, uint64_t, SimpleError**));
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
    WillOnce(DoAll(testing::SetArgPointee<2>(static_cast<SimpleError*>(nullptr)), testing::Return(nullptr)));

    data_broker->GetNext(&fi, &data);
}



TEST_F(GetDataFromFileTests, GetNextReturnsDataAndInfo) {
    EXPECT_CALL(mock, GetDataFromFileProxy(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(nullptr), testing::Return(new uint8_t[1] {'1'})));

    data_broker->GetNext(&fi, &data);

    ASSERT_THAT(data[0], Eq('1'));
    ASSERT_THAT(fi.base_name, Eq("1"));

}



TEST_F(GetDataFromFileTests, GetNextReturnsErrorWhenCannotReadData) {
    EXPECT_CALL(mock, GetDataFromFileProxy(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(new SimpleError(hidra2::IOErrors::kReadError)), testing::Return(nullptr)));

    auto err = data_broker->GetNext(&fi, &data);

    ASSERT_THAT(err->Explain(), Eq(hidra2::IOErrors::kReadError));
}


TEST_F(GetDataFromFileTests, GetNextReturnsErrorWhenCannotAllocateData) {
    EXPECT_CALL(mock, GetDataFromFileProxy(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(new SimpleError(hidra2::IOErrors::kMemoryAllocationError)),
                   testing::Return(nullptr)));

    auto err = data_broker->GetNext(&fi, &data);

    ASSERT_THAT(err->Explain(), Eq(hidra2::IOErrors::kMemoryAllocationError));
}


}
