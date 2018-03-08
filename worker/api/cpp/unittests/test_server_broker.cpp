#include <gmock/gmock.h>
#include "gtest/gtest.h"

#include "worker/data_broker.h"
#include "system_wrappers/io.h"
#include "system_wrappers/system_io.h"
#include "../src/server_data_broker.h"
#include "../src/curl_http_client.h"
#include "unittests/MockIO.h"
#include "MockHttpClient.h"
#include "../src/http_error.h"

using hidra2::DataBrokerFactory;
using hidra2::DataBroker;
using hidra2::ServerDataBroker;
using hidra2::IO;
using hidra2::FileInfo;
using hidra2::FileData;
using hidra2::MockIO;
using hidra2::MockHttpClient;
using hidra2::HttpCode;
using hidra2::HttpError;
using hidra2::SimpleError;

using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Ne;
using ::testing::Test;
using ::testing::_;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetArgReferee;

namespace {

TEST(FolderDataBroker, SetCorrectIo) {
    auto data_broker = std::unique_ptr<ServerDataBroker> {new ServerDataBroker("test", "dbname")};
    ASSERT_THAT(dynamic_cast<hidra2::SystemIO*>(data_broker->io__.get()), Ne(nullptr));
}

TEST(FolderDataBroker, SetCorrectHttpClient) {
    auto data_broker = std::unique_ptr<ServerDataBroker> {new ServerDataBroker("test", "dbname")};
    ASSERT_THAT(dynamic_cast<hidra2::CurlHttpClient*>(data_broker->httpclient__.get()), Ne(nullptr));
}


class ServerDataBrokerTests : public Test {
  public:
    std::unique_ptr<ServerDataBroker> data_broker;
    NiceMock<MockIO> mock_io;
    NiceMock<MockHttpClient> mock_http_client;
    FileInfo info;

    void SetUp() override {
        data_broker = std::unique_ptr<ServerDataBroker> {new ServerDataBroker("test", "dbname")};
        data_broker->io__ = std::unique_ptr<IO> {&mock_io};
        data_broker->httpclient__ = std::unique_ptr<hidra2::HttpClient> {&mock_http_client};
    }
    void TearDown() override {
        data_broker->io__.release();
        data_broker->httpclient__.release();
    }
    void MockGet(const std::string& responce) {
        EXPECT_CALL(mock_http_client, Get_t(_, _, _)).WillOnce(DoAll(
                    SetArgPointee<1>(HttpCode::OK),
                    SetArgPointee<2>(nullptr),
                    Return(responce)
                ));
    }

};

TEST_F(ServerDataBrokerTests, CanConnect) {
    auto return_code = data_broker->Connect();
    ASSERT_THAT(return_code, Eq(nullptr));
}

TEST_F(ServerDataBrokerTests, GetNextReturnsErrorOnWrongInput) {
    auto return_code = data_broker->GetNext(nullptr, nullptr);
    ASSERT_THAT(return_code->Explain(), Eq(hidra2::WorkerErrorMessage::kWrongInput));
}


TEST_F(ServerDataBrokerTests, GetNextUsesCorrectUri) {
    EXPECT_CALL(mock_http_client, Get_t("test/database/dbname/next", _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::OK),
                SetArgPointee<2>(nullptr),
                Return("")));
    data_broker->GetNext(&info, nullptr);
}

TEST_F(ServerDataBrokerTests, GetNextReturnsErrorFromHttpClient) {
    EXPECT_CALL(mock_http_client, Get_t(_, _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::NotFound),
                SetArgPointee<2>(nullptr),
                Return("")));

    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err->Explain(), HasSubstr(hidra2::WorkerErrorMessage::kSourceNotFound));
    ASSERT_THAT(err->GetErrorType(), hidra2::ErrorType::kHttpError);
    ASSERT_THAT(dynamic_cast<HttpError*>(err.get())->GetCode(), Eq(HttpCode::NotFound));
}

TEST_F(ServerDataBrokerTests, GetNextReturnsEOFFromHttpClient) {
    EXPECT_CALL(mock_http_client, Get_t(_, _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::NoContent),
                SetArgPointee<2>(nullptr),
                Return("")));

    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err->Explain(), HasSubstr(hidra2::WorkerErrorMessage::kNoData));
    ASSERT_THAT(err->GetErrorType(), hidra2::ErrorType::kEndOfFile);
}


FileInfo CreateFI() {
    FileInfo fi;
    fi.size = 100;
    fi.id = 1;
    fi.name = "name";
    fi.modify_date = std::chrono::system_clock::now();
    return fi;
}

TEST_F(ServerDataBrokerTests, GetNextReturnsFileInfo) {
    auto to_send = CreateFI();
    auto json = to_send.Json();

    MockGet(json);

    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err, Eq(nullptr));

    ASSERT_THAT(info.name, Eq(to_send.name));
    ASSERT_THAT(info.size, Eq(to_send.size));
    ASSERT_THAT(info.id, Eq(to_send.id));
    ASSERT_THAT(info.modify_date, Eq(to_send.modify_date));
}


TEST_F(ServerDataBrokerTests, GetNextReturnsParseError) {
    MockGet("error_responce");
    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err->Explain(), Eq(hidra2::WorkerErrorMessage::kErrorReadingSource));
}


TEST_F(ServerDataBrokerTests, GetNextReturnsIfNoDtataNeeded) {
    MockGet("error_responce");
    EXPECT_CALL( mock_io, GetDataFromFile_t(_, _, _)).Times(0);

    data_broker->GetNext(&info, nullptr);
}

TEST_F(ServerDataBrokerTests, GetNextCallsReadFromFile) {
    auto to_send = CreateFI();
    auto json = to_send.Json();

    MockGet(json);

    EXPECT_CALL(mock_io, GetDataFromFile_t("name", 100, _)).
    WillOnce(DoAll(SetArgPointee<2>(new hidra2::SimpleError{"s"}), testing::Return(nullptr)));

    FileData data;
    data_broker->GetNext(&info, &data);

}

}
