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
#include "mocking.h"
#include "../src/tcp_client.h"

using asapo::DataBrokerFactory;
using asapo::DataBroker;
using asapo::ServerDataBroker;
using asapo::IO;
using asapo::FileInfo;
using asapo::FileData;
using asapo::MockIO;
using asapo::MockHttpClient;
using asapo::MockNetClient;
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

TEST(FolderDataBroker, Constructor) {
    auto data_broker =
    std::unique_ptr<ServerDataBroker> {new ServerDataBroker("test", "path", "beamtime_id", "token")};
    ASSERT_THAT(dynamic_cast<asapo::SystemIO*>(data_broker->io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::CurlHttpClient*>(data_broker->httpclient__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::TcpClient*>(data_broker->net_client__.get()), Ne(nullptr));
}

class ServerDataBrokerTests : public Test {
  public:
    std::unique_ptr<ServerDataBroker> data_broker;
    NiceMock<MockIO> mock_io;
    NiceMock<MockHttpClient> mock_http_client;
    NiceMock<MockNetClient> mock_netclient;
    FileInfo info;
    std::string expected_server_uri = "test:8400";
    std::string expected_broker_uri = "broker:5005";
    std::string expected_token = "token";
    std::string expected_path = "/tmp/beamline/beamtime";
    std::string expected_filename = "filename";
    std::string expected_full_path = std::string("/tmp/beamline/beamtime") + asapo::kPathSeparator + expected_filename;
    std::string expected_group_id = "groupid";
    uint64_t expected_dataset_id = 1;
    static const uint64_t expected_buf_id = 123;
    void SetUp() override {
        data_broker = std::unique_ptr<ServerDataBroker> {
            new ServerDataBroker(expected_server_uri, expected_path, "beamtime_id", expected_token)
        };
        data_broker->io__ = std::unique_ptr<IO> {&mock_io};
        data_broker->httpclient__ = std::unique_ptr<asapo::HttpClient> {&mock_http_client};
        data_broker->net_client__ = std::unique_ptr<asapo::NetClient> {&mock_netclient};
    }
    void TearDown() override {
        data_broker->io__.release();
        data_broker->httpclient__.release();
        data_broker->net_client__.release();
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
    void MockReadDataFromFile(int times = 1) {
        if (times == 0) {
            EXPECT_CALL(mock_io, GetDataFromFile_t(_, _, _)).Times(0);
            return;
        }

        EXPECT_CALL(mock_io, GetDataFromFile_t(expected_full_path, testing::Pointee(100), _)).Times(times).
        WillRepeatedly(DoAll(SetArgPointee<2>(new asapo::SimpleError{"s"}), testing::Return(nullptr)));
    }
    FileInfo CreateFI(uint64_t buf_id = expected_buf_id) {
        FileInfo fi;
        fi.size = 100;
        fi.id = 1;
        fi.buf_id = buf_id;
        fi.name = expected_filename;
        fi.modify_date = std::chrono::system_clock::now();
        return fi;
    }
};

TEST_F(ServerDataBrokerTests, CanConnect) {
    auto return_code = data_broker->Connect();
    ASSERT_THAT(return_code, Eq(nullptr));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsErrorOnWrongInput) {
    auto return_code = data_broker->GetNext(nullptr, "", nullptr);
    ASSERT_THAT(return_code->Explain(), Eq(asapo::WorkerErrorMessage::kWrongInput));
}

TEST_F(ServerDataBrokerTests, GetNextUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_group_id + "/next?token="
                                        + expected_token, _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("")));
    data_broker->GetNext(&info, expected_group_id, nullptr);
}

TEST_F(ServerDataBrokerTests, GetLastUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_group_id + "/last?token="
                                        + expected_token, _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("")));
    data_broker->GetLast(&info, expected_group_id, nullptr);
}

TEST_F(ServerDataBrokerTests, GetImageReturnsEOFFromHttpClient) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                SetArgPointee<2>(nullptr),
                Return("{\"id\":1}")));

    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), HasSubstr("timeout"));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsNotAuthorized) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Unauthorized),
                SetArgPointee<2>(nullptr),
                Return("")));

    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), HasSubstr("authorization"));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsWrongResponseFromHttpClient) {

    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                SetArgPointee<2>(nullptr),
                Return("id")));

    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err->Explain(), HasSubstr("Cannot parse"));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsIfBrokerAddressNotFound) {
    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/discovery/broker"), _,
                                        _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
                                                    SetArgPointee<1>(HttpCode::NotFound),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("")));

    data_broker->SetTimeout(100);
    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err->Explain(), AllOf(HasSubstr("broker uri"), HasSubstr("cannot")));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsIfBrokerUriEmpty) {
    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/discovery/broker"), _,
                                        _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
                                                    SetArgPointee<1>(HttpCode::OK),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("")));

    data_broker->SetTimeout(100);
    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err->Explain(), AllOf(HasSubstr("broker uri"), HasSubstr("cannot")));
}

TEST_F(ServerDataBrokerTests, GetDoNotCallBrokerUriIfAlreadyFound) {
    MockGetBrokerUri();
    MockGet("error_response");

    data_broker->SetTimeout(100);
    data_broker->GetNext(&info, expected_group_id, nullptr);
    Mock::VerifyAndClearExpectations(&mock_http_client);

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/discovery/broker"), _, _)).Times(0);
    MockGet("error_response");
    data_broker->GetNext(&info, expected_group_id, nullptr);
}

TEST_F(ServerDataBrokerTests, GetBrokerUriAgainAfterConnectionError) {
    MockGetBrokerUri();
    MockGetError();

    data_broker->SetTimeout(0);
    data_broker->GetNext(&info, expected_group_id, nullptr);
    Mock::VerifyAndClearExpectations(&mock_http_client);

    MockGetBrokerUri();
    MockGet("error_response");
    data_broker->GetNext(&info, expected_group_id, nullptr);
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
    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err->Explain(), HasSubstr("timeout"));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsFileInfo) {
    MockGetBrokerUri();

    auto to_send = CreateFI();
    auto json = to_send.Json();

    MockGet(json);

    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(nullptr));

    ASSERT_THAT(info.name, Eq(to_send.name));
    ASSERT_THAT(info.size, Eq(to_send.size));
    ASSERT_THAT(info.id, Eq(to_send.id));
    ASSERT_THAT(info.modify_date, Eq(to_send.modify_date));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsParseError) {
    MockGetBrokerUri();
    MockGet("error_response");
    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err->Explain(), Eq(asapo::WorkerErrorMessage::kErrorReadingSource));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsIfNoDtataNeeded) {
    MockGetBrokerUri();
    MockGet("error_response");

    EXPECT_CALL(mock_netclient, GetData_t(_, _)).Times(0);
    EXPECT_CALL(mock_io, GetDataFromFile_t(_, _, _)).Times(0);

    data_broker->GetNext(&info, expected_group_id, nullptr);
}

TEST_F(ServerDataBrokerTests, GetImageTriesToGetDataFromMemoryCache) {
    MockGetBrokerUri();
    auto to_send = CreateFI();
    auto json = to_send.Json();
    MockGet(json);
    FileData data;

    EXPECT_CALL(mock_netclient, GetData_t(&info, &data)).WillOnce(Return(nullptr));
    MockReadDataFromFile(0);

    data_broker->GetNext(&info, expected_group_id, &data);

    ASSERT_THAT(info.buf_id, Eq(expected_buf_id));

}

TEST_F(ServerDataBrokerTests, GetImageCallsReadFromFileIfCannotReadFromCache) {
    MockGetBrokerUri();
    auto to_send = CreateFI();
    auto json = to_send.Json();
    MockGet(json);

    FileData data;

    EXPECT_CALL(mock_netclient, GetData_t(&info,
                                          &data)).WillOnce(Return(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()));
    MockReadDataFromFile();

    data_broker->GetNext(&info, expected_group_id, &data);
    ASSERT_THAT(info.buf_id, Eq(0));
}

TEST_F(ServerDataBrokerTests, GetImageCallsReadFromFileIfZeroBufId) {
    MockGetBrokerUri();
    auto to_send = CreateFI(0);
    auto json = to_send.Json();
    MockGet(json);

    FileData data;


    EXPECT_CALL(mock_netclient, GetData_t(_, _)).Times(0);

    MockReadDataFromFile();

    data_broker->GetNext(&info, expected_group_id, &data);
}


TEST_F(ServerDataBrokerTests, GenerateNewGroupIdReturnsErrorCreateGroup) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Post_t(HasSubstr("creategroup"), "", _, _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
                SetArgPointee<2>(HttpCode::BadRequest),
                SetArgPointee<3>(nullptr),
                Return("")));

    data_broker->SetTimeout(100);
    asapo::Error err;
    auto groupid = data_broker->GenerateNewGroupId(&err);
    if (err != nullptr ) {
        ASSERT_THAT(err->Explain(), HasSubstr("timeout"));
    }
    ASSERT_THAT(groupid, Eq(""));
}


TEST_F(ServerDataBrokerTests, GenerateNewGroupIdReturnsGroupID) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Post_t(expected_broker_uri + "/creategroup?token=" + expected_token, "", _,
                                         _)).WillOnce(DoAll(
                                                 SetArgPointee<2>(HttpCode::OK),
                                                 SetArgPointee<3>(nullptr),
                                                 Return(expected_group_id)));

    data_broker->SetTimeout(100);
    asapo::Error err;
    auto groupid = data_broker->GenerateNewGroupId(&err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(groupid, Eq(expected_group_id));
}

TEST_F(ServerDataBrokerTests, ResetCounterUsesCorrectUri) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_group_id +
                                         "/resetcounter?token="
                                         + expected_token, _, _, _)).WillOnce(DoAll(
                                                     SetArgPointee<2>(HttpCode::OK),
                                                     SetArgPointee<3>(nullptr),
                                                     Return("")));
    auto err = data_broker->ResetCounter(expected_group_id);
    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(ServerDataBrokerTests, GetNDataSetsUsesCorrectUri) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/size?token="
                                        + expected_token, _, _)).WillOnce(DoAll(
                                                    SetArgPointee<1>(HttpCode::OK),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("{\"size\":10}")));
    asapo::Error err;
    auto size = data_broker->GetNDataSets(&err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(size, Eq(10));
}


TEST_F(ServerDataBrokerTests, GetNDataSetsErrorOnWrongResponce) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/size?token="
                                        + expected_token, _, _)).WillRepeatedly(DoAll(
                                                    SetArgPointee<1>(HttpCode::Unauthorized),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("")));
    asapo::Error err;
    auto size = data_broker->GetNDataSets(&err);
    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(size, Eq(0));
}


TEST_F(ServerDataBrokerTests, GetNDataErrorOnWrongParse) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/size?token="
                                        + expected_token, _, _)).WillOnce(DoAll(
                                                    SetArgPointee<1>(HttpCode::OK),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("{\"siz\":10}")));
    asapo::Error err;
    auto size = data_broker->GetNDataSets(&err);
    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(size, Eq(0));
}

TEST_F(ServerDataBrokerTests, GetByIdUsesCorrectUri) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);
    auto to_send = CreateFI();
    auto json = to_send.Json();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/"  + expected_group_id
                                            + "/" + std::to_string(
                                            expected_dataset_id) + "?token="
                                        + expected_token+"&reset=true", _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return(json)));

    auto err = data_broker->GetById(expected_dataset_id, &info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(info.name, Eq(to_send.name));

}

TEST_F(ServerDataBrokerTests, GetByIdReturnsNoData) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);
    auto to_send = CreateFI();
    auto json = to_send.Json();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_group_id
                                            + "/" + std::to_string(
                                            expected_dataset_id) + "?token="
                                        + expected_token+"&reset=true", _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::Conflict),
                                                SetArgPointee<2>(nullptr),
                                                Return("{\"id\":1}")));

    auto err = data_broker->GetById(expected_dataset_id, &info, expected_group_id, nullptr);

    ASSERT_THAT(err->GetErrorType(), Eq(asapo::ErrorType::kEndOfFile));

}



}
