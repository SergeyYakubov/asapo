#include <gmock/gmock.h>
#include "gtest/gtest.h"

#include "worker/data_broker.h"
#include "io/io.h"
#include "../../../../common/cpp/src/system_io/system_io.h"
#include "../src/server_data_broker.h"
#include "../../../../common/cpp/src/http_client/curl_http_client.h"
#include "unittests/MockIO.h"
#include "unittests/MockHttpClient.h"
#include "http_client/http_error.h"

using asapo::DataBrokerFactory;
using asapo::DataBroker;
using asapo::ServerDataBroker;
using asapo::IO;
using asapo::FileInfo;
using asapo::FileData;
using asapo::MockIO;
using asapo::MockHttpClient;
using asapo::HttpCode;
using asapo::HttpError;
using asapo::SimpleError;

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
using testing::AllOf;

namespace {

TEST(FolderDataBroker, SetCorrectIo) {
    auto data_broker = std::unique_ptr<ServerDataBroker> {new ServerDataBroker("test", "beamtime_id", "token")};
    ASSERT_THAT(dynamic_cast<asapo::SystemIO*>(data_broker->io__.get()), Ne(nullptr));
}

TEST(FolderDataBroker, SetCorrectHttpClient) {
    auto data_broker = std::unique_ptr<ServerDataBroker> {new ServerDataBroker("test", "beamtime_id", "token")};
    ASSERT_THAT(dynamic_cast<asapo::CurlHttpClient*>(data_broker->httpclient__.get()), Ne(nullptr));
}


class ServerDataBrokerTests : public Test {
  public:
    std::unique_ptr<ServerDataBroker> data_broker;
    NiceMock<MockIO> mock_io;
    NiceMock<MockHttpClient> mock_http_client;
    FileInfo info;
    std::string expected_server_uri = "test:8400";
    std::string expected_broker_uri = "broker:5005";
    std::string expected_token = "token";

    void SetUp() override {
        data_broker = std::unique_ptr<ServerDataBroker> {new ServerDataBroker(expected_server_uri, "beamtime_id", expected_token)};
        data_broker->io__ = std::unique_ptr<IO> {&mock_io};
        data_broker->httpclient__ = std::unique_ptr<asapo::HttpClient> {&mock_http_client};
    }
    void TearDown() override {
        data_broker->io__.release();
        data_broker->httpclient__.release();
    }
    void MockGet(const std::string& response) {
        EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_broker_uri), _, _)).WillOnce(DoAll(
                    SetArgPointee<1>(HttpCode::OK),
                    SetArgPointee<2>(nullptr),
                    Return(response)
                ));
    }

    void MockGetError() {
        EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_broker_uri), _, _)).WillOnce(DoAll(
                    SetArgPointee<1>(HttpCode::NotFound),
                    SetArgPointee<2>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
                    Return("")
                ));
    }

    void MockGetBrokerUri() {
        EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/discovery/broker"), _, _)).WillOnce(DoAll(
                    SetArgPointee<1>(HttpCode::OK),
                    SetArgPointee<2>(nullptr),
                    Return(expected_broker_uri)));
    }

};

TEST_F(ServerDataBrokerTests, CanConnect) {
    auto return_code = data_broker->Connect();
    ASSERT_THAT(return_code, Eq(nullptr));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsErrorOnWrongInput) {
    auto return_code = data_broker->GetNext(nullptr, nullptr);
    ASSERT_THAT(return_code->Explain(), Eq(asapo::WorkerErrorMessage::kWrongInput));
}


TEST_F(ServerDataBrokerTests, GetNextUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/next?token=" + expected_token, _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("")));
    data_broker->GetNext(&info, nullptr);
}

TEST_F(ServerDataBrokerTests, GetLastUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/last?token=" + expected_token, _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("")));
    data_broker->GetLast(&info, nullptr);
}


TEST_F(ServerDataBrokerTests, GetImageReturnsEOFFromHttpClient) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                SetArgPointee<2>(nullptr),
                Return("{\"id\":1}")));

    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), HasSubstr("timeout"));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsNotAuthorized) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Unauthorized),
                SetArgPointee<2>(nullptr),
                Return("")));

    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), HasSubstr("authorization"));
}


TEST_F(ServerDataBrokerTests, GetImageReturnsWrongResponseFromHttpClient) {

    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                SetArgPointee<2>(nullptr),
                Return("id")));

    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err->Explain(), HasSubstr("Cannot parse"));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsIfBrokerAddressNotFound) {
    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/discovery/broker"), _,
                                        _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
                                                    SetArgPointee<1>(HttpCode::NotFound),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("")));

    data_broker->SetTimeout(100);
    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err->Explain(), AllOf(HasSubstr("broker uri"), HasSubstr("cannot")));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsIfBrokerUriEmpty) {
    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/discovery/broker"), _,
                                        _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
                                                    SetArgPointee<1>(HttpCode::OK),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("")));

    data_broker->SetTimeout(100);
    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err->Explain(), AllOf(HasSubstr("broker uri"), HasSubstr("cannot")));
}



TEST_F(ServerDataBrokerTests, GetDoNotCallBrokerUriIfAlreadyFound) {
    MockGetBrokerUri();
    MockGet("error_response");

    data_broker->SetTimeout(100);
    data_broker->GetNext(&info, nullptr);
    Mock::VerifyAndClearExpectations(&mock_http_client);

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/discovery/broker"), _, _)).Times(0);
    MockGet("error_response");
    data_broker->GetNext(&info, nullptr);
}


TEST_F(ServerDataBrokerTests, GetBrokerUriAgainAfterConnectionError) {
    MockGetBrokerUri();
    MockGetError();

    data_broker->SetTimeout(0);
    data_broker->GetNext(&info, nullptr);
    Mock::VerifyAndClearExpectations(&mock_http_client);

    MockGetBrokerUri();
    MockGet("error_response");
    data_broker->GetNext(&info, nullptr);
}


TEST_F(ServerDataBrokerTests, GetImageReturnsEOFFromHttpClientUntilTimeout) {
    MockGetBrokerUri();


    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                SetArgPointee<2>(nullptr),
                Return("{\"id\":1}")));

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/1?token=" + expected_token, _,
                                        _)).Times(AtLeast(1)).WillRepeatedly(DoAll(
                                                    SetArgPointee<1>(HttpCode::Conflict),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("{\"id\":1}")));


    data_broker->SetTimeout(100);
    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err->Explain(), HasSubstr("timeout"));
}



FileInfo CreateFI() {
    FileInfo fi;
    fi.size = 100;
    fi.id = 1;
    fi.name = "name";
    fi.modify_date = std::chrono::system_clock::now();
    return fi;
}

TEST_F(ServerDataBrokerTests, GetImageReturnsFileInfo) {
    MockGetBrokerUri();

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


TEST_F(ServerDataBrokerTests, GetImageReturnsParseError) {
    MockGetBrokerUri();
    MockGet("error_response");
    auto err = data_broker->GetNext(&info, nullptr);

    ASSERT_THAT(err->Explain(), Eq(asapo::WorkerErrorMessage::kErrorReadingSource));
}


TEST_F(ServerDataBrokerTests, GetImageReturnsIfNoDtataNeeded) {
    MockGetBrokerUri();
    MockGet("error_response");
    EXPECT_CALL( mock_io, GetDataFromFile_t(_, _, _)).Times(0);

    data_broker->GetNext(&info, nullptr);
}

TEST_F(ServerDataBrokerTests, GetImageCallsReadFromFile) {
    MockGetBrokerUri();
    auto to_send = CreateFI();
    auto json = to_send.Json();
    MockGet(json);

    EXPECT_CALL(mock_io, GetDataFromFile_t("name", testing::Pointee(100), _)).
    WillOnce(DoAll(SetArgPointee<2>(new asapo::SimpleError{"s"}), testing::Return(nullptr)));

    FileData data;
    data_broker->GetNext(&info, &data);

}

}
