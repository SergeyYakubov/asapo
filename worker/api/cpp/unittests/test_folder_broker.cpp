#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "gtest/gtest.h"

#include "worker/data_broker.h"
#include "io/io.h"
#include "../../../../common/cpp/src/system_io/system_io.h"
#include "../src/folder_data_broker.h"

using asapo::DataBrokerFactory;
using asapo::DataBroker;
using asapo::FolderDataBroker;
using asapo::IO;
using asapo::FileInfos;
using asapo::FileInfo;
using asapo::FileData;
using asapo::Error;
using asapo::TextError;
using asapo::SimpleError;

using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;
using ::testing::_;
using ::testing::Mock;


namespace {

TEST(FolderDataBroker, SetCorrectIO) {
    auto data_broker = new FolderDataBroker("test");
    ASSERT_THAT(dynamic_cast<asapo::SystemIO*>(data_broker->io__.get()), Ne(nullptr));
    delete data_broker;
}

class FakeIO: public asapo::MockIO {
  public:

    virtual std::string ReadFileToString(const std::string& fname, Error* err) const noexcept override {
        return "OK";
    }

    FileInfos FilesInFolder(const std::string& folder, Error* err) const override {
        *err = nullptr;
        FileInfos file_infos;
        FileInfo fi;
        fi.size = 100;
        fi.name = "1";
        file_infos.push_back(fi);
        fi.name = "2";
        file_infos.push_back(fi);
        fi.name = "3";
        file_infos.push_back(fi);
        return file_infos;
    }
};

class IOFolderNotFound: public FakeIO {
  public:
    FileInfos FilesInFolder(const std::string& folder, Error* err) const override {
        *err = asapo::IOErrorTemplates::kFileNotFound.Generate();
        return {};
    }
};

class IOFolderUnknownError: public FakeIO {
  public:
    FileInfos FilesInFolder(const std::string& folder, Error* err) const override {
        *err  = asapo::IOErrorTemplates::kUnknownIOError.Generate();
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
        *err = asapo::IOErrorTemplates::kPermissionDenied.Generate();
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

    ASSERT_THAT(return_code->Explain(), Eq(asapo::WorkerErrorMessage::kSourceAlreadyConnected));
}


TEST_F(FolderDataBrokerTests, CannotConnectWhenNoFolder) {
    data_broker->io__ = std::unique_ptr<IO> {new IOFolderNotFound()};

    auto return_code = data_broker->Connect();

    ASSERT_THAT(return_code->Explain(), Eq(asapo::IOErrorTemplates::kFileNotFound.Generate()->Explain()));
}

TEST_F(FolderDataBrokerTests, ConnectReturnsUnknownIOError) {
    data_broker->io__ = std::unique_ptr<IO> {new IOFolderUnknownError()};

    auto return_code = data_broker->Connect();

    ASSERT_THAT(return_code->Explain(), Eq(asapo::IOErrorTemplates::kUnknownIOError.Generate()->Explain()));
}

TEST_F(FolderDataBrokerTests, GetNextWithoutConnectReturnsError) {
    auto err = data_broker->GetNext(nullptr, nullptr);

    ASSERT_THAT(err->Explain(), Eq(asapo::WorkerErrorMessage::kSourceNotConnected));
}

TEST_F(FolderDataBrokerTests, GetNextWithNullPointersReturnsError) {
    data_broker->Connect();

    auto err = data_broker->GetNext(nullptr, nullptr);

    ASSERT_THAT(err->Explain(), Eq(asapo::WorkerErrorMessage::kWrongInput));
}

TEST_F(FolderDataBrokerTests, GetNextReturnsFileInfo) {
    data_broker->Connect();
    FileInfo fi;

    auto err = data_broker->GetNext(&fi, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(fi.name, Eq("1"));
    ASSERT_THAT(fi.size, Eq(100));

}

TEST_F(FolderDataBrokerTests, SecondNextReturnsAnotherFileInfo) {
    data_broker->Connect();
    FileInfo fi;
    data_broker->GetNext(&fi, nullptr);

    auto err = data_broker->GetNext(&fi, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(fi.name, Eq("2"));
}

TEST_F(FolderDataBrokerTests, GetNextFromEmptyFolderReturnsError) {
    data_broker->io__ = std::unique_ptr<IO> {new IOEmptyFolder()};
    data_broker->Connect();
    FileInfo fi;

    auto err = data_broker->GetNext(&fi, nullptr);
    ASSERT_TRUE(asapo::ErrorTemplates::kEndOfFile == err);

    ASSERT_THAT(err->Explain(), Eq(asapo::WorkerErrorMessage::kNoData));
}

TEST_F(FolderDataBrokerTests, GetNextReturnsErrorWhenFilePermissionsDenied) {
    data_broker->io__ = std::unique_ptr<IO> {new IOCannotOpenFile()};
    data_broker->Connect();
    FileInfo fi;
    FileData data;

    auto err = data_broker->GetNext(&fi, &data);
    ASSERT_THAT(err->Explain(), Eq(asapo::IOErrorTemplates::kPermissionDenied.Generate()->Explain()));
}


class OpenFileMock : public FakeIO {
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
    EXPECT_CALL(mock, GetDataFromFile_t("/path/to/file/1", _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(static_cast<SimpleError*>(nullptr)), testing::Return(nullptr)));

    data_broker->GetNext(&fi, &data);
}

TEST_F(GetDataFromFileTests, GetNextReturnsDataAndInfo) {
    EXPECT_CALL(mock, GetDataFromFile_t(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(nullptr), testing::Return(new uint8_t[1] {'1'})));

    data_broker->GetNext(&fi, &data);

    ASSERT_THAT(data[0], Eq('1'));
    ASSERT_THAT(fi.name, Eq("1"));

}

TEST_F(GetDataFromFileTests, GetNextReturnsErrorWhenCannotReadData) {
    EXPECT_CALL(mock, GetDataFromFile_t(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(asapo::IOErrorTemplates::kReadError.Generate().release()),
                   testing::Return(nullptr)));

    auto err = data_broker->GetNext(&fi, &data);

    ASSERT_THAT(err->Explain(), Eq(asapo::IOErrorTemplates::kReadError.Generate()->Explain()));
}

TEST_F(GetDataFromFileTests, GetNextReturnsErrorWhenCannotAllocateData) {
    EXPECT_CALL(mock, GetDataFromFile_t(_, _, _)).
    WillOnce(DoAll(testing::SetArgPointee<2>(asapo::ErrorTemplates::kMemoryAllocationError.Generate().release()),
                   testing::Return(nullptr)));

    auto err = data_broker->GetNext(&fi, &data);

    ASSERT_THAT(err->Explain(), Eq(asapo::ErrorTemplates::kMemoryAllocationError.Generate()->Explain()));
}


}
