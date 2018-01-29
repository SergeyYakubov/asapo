#include <gmock/gmock.h>
#include "gtest/gtest.h"

#include "worker/data_broker.h"
#include "system_wrappers/io.h"
#include "system_wrappers/system_io.h"
#include "../src/server_data_broker.h"
#include "../src/curl_http_client.h"
#include "unittests/MockIO.h"
#include "MockHttpClient.h"

using hidra2::DataBrokerFactory;
using hidra2::DataBroker;
using hidra2::ServerDataBroker;
using hidra2::WorkerErrorCode;
using hidra2::IO;
using hidra2::IOErrors;
using hidra2::FileInfo;
using hidra2::FileData;
using hidra2::MockIO;
using hidra2::MockHttpClient;

using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;
using ::testing::_;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgPointee;


namespace {

TEST(FolderDataBroker, SetCorrectMembers) {
    auto data_broker = std::unique_ptr<ServerDataBroker> {new ServerDataBroker("test", "database")};
    ASSERT_THAT(dynamic_cast<hidra2::SystemIO*>(data_broker->io__.get()), Ne(nullptr));
}

TEST(FolderDataBroker, SetCorrectHttpClient) {
    auto data_broker = std::unique_ptr<ServerDataBroker> {new ServerDataBroker("test", "database")};
}


class ServerDataBrokerTests : public Test {
  public:
    std::unique_ptr<ServerDataBroker> data_broker;
    NiceMock<MockIO> mock_io;
    NiceMock<MockHttpClient> mock_http_client;
    FileInfo info;

    void SetUp() override {
        data_broker = std::unique_ptr<ServerDataBroker> {new ServerDataBroker("test", "database")};
        data_broker->io__ = std::unique_ptr<IO> {&mock_io};
        data_broker->httpclient__ = std::unique_ptr<hidra2::HttpClient> {&mock_http_client};
    }
    void TearDown() override {
        data_broker->io__.release();
        data_broker->httpclient__.release();
    }
};

TEST_F(ServerDataBrokerTests, CanConnect) {
    auto return_code = data_broker->Connect();
    ASSERT_THAT(return_code, Eq(WorkerErrorCode::kOK));
}

TEST_F(ServerDataBrokerTests, GetNextReturnsErrorOnWrongInput) {
    auto return_code = data_broker->GetNext(nullptr, nullptr);
    ASSERT_THAT(return_code, Eq(WorkerErrorCode::kWrongInput));
}

TEST_F(ServerDataBrokerTests, GetNextUsesCorrectUri) {
    EXPECT_CALL(mock_http_client, Get("test/next?database=database", _)).WillOnce(DoAll(
                SetArgPointee<1>(WorkerErrorCode::kOK),
                Return("")));
    data_broker->GetNext(&info, nullptr);
}

TEST_F(ServerDataBrokerTests, GetNextReturnsErrorFromHttpClient) {
    EXPECT_CALL(mock_http_client, Get(_, _)).WillOnce(DoAll(
            SetArgPointee<1>(WorkerErrorCode::kSourceNotFound),
            Return("")));

    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err, Eq(WorkerErrorCode::kSourceNotFound));

}

FileInfo CreateFI() {
    FileInfo fi;
    fi.size = 100;
    fi.id = 1;
    fi.relative_path = "relative_path";
    fi.base_name = "base_name";
    fi.modify_date = std::chrono::system_clock::now();
    return fi;
}

TEST_F(ServerDataBrokerTests, GetNextReturnsFileInfo) {
    auto to_send = CreateFI();
    auto json = to_send.Json();
    EXPECT_CALL(mock_http_client, Get(_, _)).WillOnce(DoAll(
            SetArgPointee<1>(WorkerErrorCode::kOK),
            Return(json)
                                                      ));

    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err, Eq(WorkerErrorCode::kOK));

    ASSERT_THAT(info.base_name, Eq(to_send.base_name));
    ASSERT_THAT(info.size, Eq(to_send.size));
    ASSERT_THAT(info.id, Eq(to_send.id));
    ASSERT_THAT(info.modify_date, Eq(to_send.modify_date));
    ASSERT_THAT(info.relative_path, Eq(to_send.relative_path));
}


TEST_F(ServerDataBrokerTests, GetNextReturnsParseError) {
    EXPECT_CALL(mock_http_client, Get(_, _)).WillOnce(DoAll(
            SetArgPointee<1>(WorkerErrorCode::kOK),
            Return("blabla")
                                                      ));

    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err, Eq(WorkerErrorCode::kErrorReadingSource));
}


TEST_F(ServerDataBrokerTests, GetNextReturnsIfNoDtataNeeded) {
    EXPECT_CALL(mock_http_client, Get(_, _)).WillOnce(DoAll(
            SetArgPointee<1>(WorkerErrorCode::kOK),
            Return("blabla")
                                                      ));

    EXPECT_CALL( mock_io, GetDataFromFile_t(_, _, _)).Times(0);
    data_broker->GetNext(&info, nullptr);
}


TEST_F(ServerDataBrokerTests, GetNextCallsReadFromFile) {
    auto to_send = CreateFI();
    auto json = to_send.Json();

    EXPECT_CALL(mock_http_client, Get(_, _)).WillOnce(DoAll(
            SetArgPointee<1>(WorkerErrorCode::kOK),
            Return(json)));


    EXPECT_CALL(mock_io, GetDataFromFile_t("relative_path/base_name", 100, _)).
    WillOnce(DoAll(SetArgPointee<2>(IOErrors::kReadError), testing::Return(nullptr)));

    FileData data;
    data_broker->GetNext(&info, &data);

}


}
