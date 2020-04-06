#include <gmock/gmock.h>
#include <gmock/gmock.h>
#include "gtest/gtest.h"

#include "consumer/data_broker.h"
#include "consumer/consumer_error.h"
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
using ::testing::DoAll;


namespace {

TEST(FolderDataBroker, Constructor) {
    auto data_broker =
    std::unique_ptr<ServerDataBroker> {new ServerDataBroker("test", "path", false,
                asapo::SourceCredentials{"beamtime_id", "", "", "token"})
    };
    ASSERT_THAT(dynamic_cast<asapo::SystemIO*>(data_broker->io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::CurlHttpClient*>(data_broker->httpclient__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::TcpClient*>(data_broker->net_client__.get()), Ne(nullptr));
}

const uint8_t expected_value = 1;

class ServerDataBrokerTests : public Test {
  public:
    std::unique_ptr<ServerDataBroker> data_broker, fts_data_broker;
    NiceMock<MockIO> mock_io;
    NiceMock<MockHttpClient> mock_http_client;
    NiceMock<MockNetClient> mock_netclient;
    FileInfo info;
    std::string expected_server_uri = "test:8400";
    std::string expected_broker_uri = "broker:5005";
    std::string expected_fts_uri = "fts:5008";
    std::string expected_token = "token";
    std::string expected_path = "/tmp/beamline/beamtime";
    std::string expected_filename = "filename";
    std::string expected_full_path = std::string("/tmp/beamline/beamtime") + asapo::kPathSeparator + expected_filename;
    std::string expected_group_id = "groupid";
    std::string expected_stream = "stream";
    std::string expected_substream = "substream";
    std::string expected_metadata = "{\"meta\":1}";
    std::string expected_query_string = "bla";
    std::string expected_folder_token = "folder_token";
    std::string expected_beamtime_id = "beamtime_id";
    uint64_t expected_image_size = 100;
    uint64_t expected_dataset_id = 1;
    static const uint64_t expected_buf_id = 123;
    std::string expected_next_substream = "nextsubstream";
    std::string expected_fts_query_string = "{\"Folder\":\"" + expected_path + "\",\"FileName\":\"" + expected_filename +
                                            "\"}";
    std::string expected_cookie = "Authorization=Bearer " + expected_folder_token;

    void AssertSingleFileTransfer();
    void SetUp() override {
        data_broker = std::unique_ptr<ServerDataBroker> {
            new ServerDataBroker(expected_server_uri, expected_path, true, asapo::SourceCredentials{expected_beamtime_id, "", expected_stream, expected_token})
        };
        fts_data_broker = std::unique_ptr<ServerDataBroker> {
            new ServerDataBroker(expected_server_uri, expected_path, false, asapo::SourceCredentials{expected_beamtime_id, "", expected_stream, expected_token})
        };
        data_broker->io__ = std::unique_ptr<IO> {&mock_io};
        data_broker->httpclient__ = std::unique_ptr<asapo::HttpClient> {&mock_http_client};
        data_broker->net_client__ = std::unique_ptr<asapo::NetClient> {&mock_netclient};
        fts_data_broker->io__ = std::unique_ptr<IO> {&mock_io};
        fts_data_broker->httpclient__ = std::unique_ptr<asapo::HttpClient> {&mock_http_client};
        fts_data_broker->net_client__ = std::unique_ptr<asapo::NetClient> {&mock_netclient};

    }
    void TearDown() override {
        data_broker->io__.release();
        data_broker->httpclient__.release();
        data_broker->net_client__.release();
        fts_data_broker->io__.release();
        fts_data_broker->httpclient__.release();
        fts_data_broker->net_client__.release();

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
    void MockGetServiceUri(std::string service, std::string result) {
        EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/discovery/" + service), _, _)).WillOnce(DoAll(
                    SetArgPointee<1>(HttpCode::OK),
                    SetArgPointee<2>(nullptr),
                    Return(result)));
    }

    void MockBeforeFTS(FileData* data);

    void MockGetFTSUri() {
        MockGetServiceUri("fts", expected_fts_uri);
    }

    void ExpectFolderToken();
    void ExpectFileTransfer(const asapo::ConsumerErrorTemplate* p_err_template);
    void ExpectRepeatedFileTransfer();

    void MockGetBrokerUri() {
        MockGetServiceUri("broker", expected_broker_uri);
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
        fi.size = expected_image_size;
        fi.id = 1;
        fi.buf_id = buf_id;
        fi.name = expected_filename;
        fi.modify_date = std::chrono::system_clock::now();
        return fi;
    }
};


TEST_F(ServerDataBrokerTests, GetImageReturnsErrorOnWrongInput) {
    auto err = data_broker->GetNext(nullptr, "", nullptr);
    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kWrongInput));
}

TEST_F(ServerDataBrokerTests, DefaultStreamIsDetector) {
    data_broker->io__.release();
    data_broker->httpclient__.release();
    data_broker->net_client__.release();
    data_broker = std::unique_ptr<ServerDataBroker> {
        new ServerDataBroker(expected_server_uri, expected_path, false, asapo::SourceCredentials{"beamtime_id", "", "", expected_token})
    };
    data_broker->io__ = std::unique_ptr<IO> {&mock_io};
    data_broker->httpclient__ = std::unique_ptr<asapo::HttpClient> {&mock_http_client};
    data_broker->net_client__ = std::unique_ptr<asapo::NetClient> {&mock_netclient};

    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/detector/default/" + expected_group_id
                                        +
                                        "/next?token="
                                        + expected_token, _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("")));

    data_broker->GetNext(&info, expected_group_id, nullptr);
}



TEST_F(ServerDataBrokerTests, GetNextUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/"
                                        + expected_group_id + "/next?token="
                                        + expected_token, _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("")));
    data_broker->GetNext(&info, expected_group_id, nullptr);
}

TEST_F(ServerDataBrokerTests, GetNextUsesCorrectUriWithSubstream) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/" +
                                        expected_substream + "/" + expected_group_id + "/next?token="
                                        + expected_token, _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("")));
    data_broker->GetNext(&info, expected_group_id, expected_substream, nullptr);
}

TEST_F(ServerDataBrokerTests, GetLastUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/" +
                                        expected_group_id + "/last?token="
                                        + expected_token, _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("")));
    data_broker->GetLast(&info, expected_group_id, nullptr);
}

TEST_F(ServerDataBrokerTests, GetImageReturnsEndOfStreamFromHttpClient) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                SetArgPointee<2>(nullptr),
                Return("{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_substream\":\"\"}")));

    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    auto err_data = static_cast<const asapo::ConsumerErrorData*>(err->GetCustomData());

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kEndOfStream));
    ASSERT_THAT(err_data->id, Eq(1));
    ASSERT_THAT(err_data->id_max, Eq(1));
    ASSERT_THAT(err_data->next_substream, Eq(""));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsStreamFinishedFromHttpClient) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                SetArgPointee<2>(nullptr),
                Return("{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_substream\":\"" + expected_next_substream + "\"}")));

    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    auto err_data = static_cast<const asapo::ConsumerErrorData*>(err->GetCustomData());

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kStreamFinished));
    ASSERT_THAT(err_data->id, Eq(1));
    ASSERT_THAT(err_data->id_max, Eq(1));
    ASSERT_THAT(err_data->next_substream, Eq(expected_next_substream));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsNoDataFromHttpClient) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                SetArgPointee<2>(nullptr),
                Return("{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":2,\"next_substream\":\"""\"}")));


    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);
    auto err_data = static_cast<const asapo::ConsumerErrorData*>(err->GetCustomData());

    ASSERT_THAT(err_data->id, Eq(1));
    ASSERT_THAT(err_data->id_max, Eq(2));
    ASSERT_THAT(err_data->next_substream, Eq(""));

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kNoData));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsNotAuthorized) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Unauthorized),
                SetArgPointee<2>(nullptr),
                Return("")));

    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kWrongInput));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsWrongResponseFromHttpClient) {

    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                SetArgPointee<2>(nullptr),
                Return("id")));

    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kInterruptedTransaction));
    ASSERT_THAT(err->Explain(), HasSubstr("malformed"));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsIfBrokerAddressNotFound) {
    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/discovery/asapo-broker"), _,
                                        _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
                                                    SetArgPointee<1>(HttpCode::NotFound),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("")));

    data_broker->SetTimeout(100);
    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err->Explain(), AllOf(HasSubstr(expected_server_uri), HasSubstr("unavailable")));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsIfBrokerUriEmpty) {
    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/discovery/asapo-broker"), _,
                                        _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
                                                    SetArgPointee<1>(HttpCode::OK),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("")));

    data_broker->SetTimeout(100);
    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err->Explain(), AllOf(HasSubstr(expected_server_uri), HasSubstr("unavailable")));
}

TEST_F(ServerDataBrokerTests, GetDoNotCallBrokerUriIfAlreadyFound) {
    MockGetBrokerUri();
    MockGet("error_response");

    data_broker->SetTimeout(100);
    data_broker->GetNext(&info, expected_group_id, nullptr);
    Mock::VerifyAndClearExpectations(&mock_http_client);

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/discovery/asap-broker"), _, _)).Times(0);
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

TEST_F(ServerDataBrokerTests, GetImageReturnsEofStreamFromHttpClientUntilTimeout) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                SetArgPointee<2>(nullptr),
                Return("{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_substream\":\"""\"}")));

    data_broker->SetTimeout(300);
    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kEndOfStream));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsNoDataAfterTimeoutEvenIfOtherErrorOccured) {
    MockGetBrokerUri();
    data_broker->SetTimeout(300);

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                SetArgPointee<2>(nullptr),
                Return("{\"op\":\"get_record_by_id\",\"id\":" + std::to_string(expected_dataset_id) +
                       ",\"id_max\":2,\"next_substream\":\"""\"}")));

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/"  +
                                        expected_group_id + "/" + std::to_string(expected_dataset_id) + "?token="
                                        + expected_token, _, _)).Times(AtLeast(1)).WillRepeatedly(DoAll(
                                                    SetArgPointee<1>(HttpCode::NotFound),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("")));


    data_broker->SetTimeout(300);
    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kNoData));
}


TEST_F(ServerDataBrokerTests, GetNextImageReturnsImmediatelyOnTransferError) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::InternalServerError),
                SetArgPointee<2>(asapo::HttpErrorTemplates::kTransferError.Generate("sss").release()),
                Return("")));

    data_broker->SetTimeout(300);
    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kInterruptedTransaction));
    ASSERT_THAT(err->Explain(), HasSubstr("sss"));
}


ACTION(AssignArg2) {
    *arg2 = asapo::HttpErrorTemplates::kConnectionError.Generate().release();
}


TEST_F(ServerDataBrokerTests, GetNextRetriesIfConnectionHttpClientErrorUntilTimeout) {
    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/discovery/asapo-broker"), _,
                                        _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
                                                    SetArgPointee<1>(HttpCode::OK),
                                                    SetArgPointee<2>(nullptr),
                                                    Return(expected_broker_uri)));

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                AssignArg2(),
                Return("")));

    data_broker->SetTimeout(300);
    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kUnavailableService));
}

TEST_F(ServerDataBrokerTests, GetNextImageReturnsImmediatelyOnFinshedSubstream) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
                SetArgPointee<1>(HttpCode::Conflict),
                SetArgPointee<2>(nullptr),
                Return("{\"op\":\"get_record_by_id\",\"id\":2,\"id_max\":2,\"next_substream\":\"next\"}")));

    data_broker->SetTimeout(300);
    auto err = data_broker->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kStreamFinished));
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

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kInterruptedTransaction));
}

TEST_F(ServerDataBrokerTests, GetImageReturnsIfNoDataNeeded) {
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

    EXPECT_CALL(mock_http_client, Post_t(HasSubstr("creategroup"), "", _, _)).WillOnce(DoAll(
                SetArgPointee<2>(HttpCode::BadRequest),
                SetArgPointee<3>(nullptr),
                Return("")));

    data_broker->SetTimeout(100);
    asapo::Error err;
    auto groupid = data_broker->GenerateNewGroupId(&err);
    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kWrongInput));
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

TEST_F(ServerDataBrokerTests, ResetCounterByDefaultUsesCorrectUri) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/" +
                                         expected_group_id +
                                         "/resetcounter?token=" + expected_token + "&value=0", _, _, _)).WillOnce(DoAll(
                                                     SetArgPointee<2>(HttpCode::OK),
                                                     SetArgPointee<3>(nullptr),
                                                     Return("")));
    auto err = data_broker->ResetLastReadMarker(expected_group_id);
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ServerDataBrokerTests, ResetCounterUsesCorrectUri) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/" +
                                         expected_group_id +
                                         "/resetcounter?token=" + expected_token + "&value=10", _, _, _)).WillOnce(DoAll(
                                                     SetArgPointee<2>(HttpCode::OK),
                                                     SetArgPointee<3>(nullptr),
                                                     Return("")));
    auto err = data_broker->SetLastReadMarker(10, expected_group_id);
    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(ServerDataBrokerTests, ResetCounterUsesCorrectUriWithSubstream) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/" +
                                         expected_substream + "/" +
                                         expected_group_id +
                                         "/resetcounter?token=" + expected_token + "&value=10", _, _, _)).WillOnce(DoAll(
                                                     SetArgPointee<2>(HttpCode::OK),
                                                     SetArgPointee<3>(nullptr),
                                                     Return("")));
    auto err = data_broker->SetLastReadMarker(10, expected_group_id, expected_substream);
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ServerDataBrokerTests, GetCurrentSizeUsesCorrectUri) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream +
                                        "/default/size?token="
                                        + expected_token, _, _)).WillOnce(DoAll(
                                                    SetArgPointee<1>(HttpCode::OK),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("{\"size\":10}")));
    asapo::Error err;
    auto size = data_broker->GetCurrentSize(&err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(size, Eq(10));
}

TEST_F(ServerDataBrokerTests, GetCurrentSizeUsesCorrectUriWithSubstream) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/" +
                                        expected_substream + "/size?token="
                                        + expected_token, _, _)).WillOnce(DoAll(
                                                    SetArgPointee<1>(HttpCode::OK),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("{\"size\":10}")));
    asapo::Error err;
    auto size = data_broker->GetCurrentSize(expected_substream, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(size, Eq(10));
}


TEST_F(ServerDataBrokerTests, GetCurrentSizeErrorOnWrongResponce) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream +
                                        "/default/size?token="
                                        + expected_token, _, _)).WillRepeatedly(DoAll(
                                                    SetArgPointee<1>(HttpCode::Unauthorized),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("")));
    asapo::Error err;
    auto size = data_broker->GetCurrentSize(&err);
    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(size, Eq(0));
}


TEST_F(ServerDataBrokerTests, GetNDataErrorOnWrongParse) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream +
                                        "/default/size?token="
                                        + expected_token, _, _)).WillOnce(DoAll(
                                                    SetArgPointee<1>(HttpCode::OK),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("{\"siz\":10}")));
    asapo::Error err;
    auto size = data_broker->GetCurrentSize(&err);
    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(size, Eq(0));
}

TEST_F(ServerDataBrokerTests, GetByIdUsesCorrectUri) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);
    auto to_send = CreateFI();
    auto json = to_send.Json();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/"  +
                                        expected_group_id
                                        + "/" + std::to_string(
                                            expected_dataset_id) + "?token="
                                        + expected_token, _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return(json)));

    auto err = data_broker->GetById(expected_dataset_id, &info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(info.name, Eq(to_send.name));
}


TEST_F(ServerDataBrokerTests, GetByIdTimeouts) {
    MockGetBrokerUri();
    data_broker->SetTimeout(10);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/"  +
                                        expected_group_id + "/" + std::to_string(expected_dataset_id) + "?token="
                                        + expected_token, _, _)).WillOnce(DoAll(
                                                    SetArgPointee<1>(HttpCode::Conflict),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("")));

    auto err = data_broker->GetById(expected_dataset_id, &info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kNoData));
}

TEST_F(ServerDataBrokerTests, GetByIdReturnsEndOfStream) {
    MockGetBrokerUri();
    data_broker->SetTimeout(10);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/"  +
                                        expected_group_id + "/" + std::to_string(expected_dataset_id) + "?token="
                                        + expected_token, _, _)).WillOnce(DoAll(
                                                    SetArgPointee<1>(HttpCode::Conflict),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_substream\":\"""\"}")));


    auto err = data_broker->GetById(expected_dataset_id, &info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kEndOfStream));
}

TEST_F(ServerDataBrokerTests, GetByIdReturnsEndOfStreamWhenIdTooLarge) {
    MockGetBrokerUri();
    data_broker->SetTimeout(10);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/"  +
                                        expected_group_id + "/" + std::to_string(expected_dataset_id) + "?token="
                                        + expected_token, _, _)).WillOnce(DoAll(
                                                    SetArgPointee<1>(HttpCode::Conflict),
                                                    SetArgPointee<2>(nullptr),
                                                    Return("{\"op\":\"get_record_by_id\",\"id\":100,\"id_max\":1,\"next_substream\":\"""\"}")));


    auto err = data_broker->GetById(expected_dataset_id, &info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kEndOfStream));
}


TEST_F(ServerDataBrokerTests, GetMetaDataOK) {
    MockGetBrokerUri();
    data_broker->SetTimeout(100);


    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream +
                                        "/default/0/meta/0?token="
                                        + expected_token, _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return(expected_metadata)));

    asapo::Error err;
    auto res = data_broker->GetBeamtimeMeta(&err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(res, Eq(expected_metadata));

}


TEST_F(ServerDataBrokerTests, QueryImagesReturnError) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Post_t(HasSubstr("queryimages"), expected_query_string, _, _)).WillOnce(DoAll(
                SetArgPointee<2>(HttpCode::BadRequest),
                SetArgPointee<3>(nullptr),
                Return("error in query")));

    data_broker->SetTimeout(1000);
    asapo::Error err;
    auto images = data_broker->QueryImages(expected_query_string, &err);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kWrongInput));
    ASSERT_THAT(err->Explain(), HasSubstr("query"));
    ASSERT_THAT(images.size(), Eq(0));
}


TEST_F(ServerDataBrokerTests, QueryImagesReturnEmptyResults) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Post_t(HasSubstr("queryimages"), expected_query_string, _, _)).WillOnce(DoAll(
                SetArgPointee<2>(HttpCode::OK),
                SetArgPointee<3>(nullptr),
                Return("[]")));

    data_broker->SetTimeout(100);
    asapo::Error err;
    auto images = data_broker->QueryImages(expected_query_string, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(images.size(), Eq(0));
}

TEST_F(ServerDataBrokerTests, QueryImagesWrongResponseArray) {

    MockGetBrokerUri();

    auto rec1 = CreateFI();
    auto rec2 = CreateFI();
    auto json1 = rec1.Json();
    auto json2 = rec2.Json();
    auto responce_string = json1 + "," + json2 + "]"; // no [ at the beginning


    EXPECT_CALL(mock_http_client, Post_t(HasSubstr("queryimages"), expected_query_string, _, _)).WillOnce(DoAll(
                SetArgPointee<2>(HttpCode::OK),
                SetArgPointee<3>(nullptr),
                Return(responce_string)));

    data_broker->SetTimeout(100);
    asapo::Error err;
    auto images = data_broker->QueryImages(expected_query_string, &err);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(images.size(), Eq(0));
    ASSERT_THAT(err->Explain(), HasSubstr("response"));
}

TEST_F(ServerDataBrokerTests, QueryImagesWrongResponseRecorsd) {

    MockGetBrokerUri();

    auto responce_string = R"([{"bla":1},{"err":}])";


    EXPECT_CALL(mock_http_client, Post_t(HasSubstr("queryimages"), expected_query_string, _, _)).WillOnce(DoAll(
                SetArgPointee<2>(HttpCode::OK),
                SetArgPointee<3>(nullptr),
                Return(responce_string)));

    data_broker->SetTimeout(100);
    asapo::Error err;
    auto images = data_broker->QueryImages(expected_query_string, &err);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(images.size(), Eq(0));
    ASSERT_THAT(err->Explain(), HasSubstr("response"));
}



TEST_F(ServerDataBrokerTests, QueryImagesReturnRecords) {

    MockGetBrokerUri();

    auto rec1 = CreateFI();
    auto rec2 = CreateFI();
    rec2.name = "ttt";
    auto json1 = rec1.Json();
    auto json2 = rec2.Json();
    auto responce_string = "[" + json1 + "," + json2 + "]";


    EXPECT_CALL(mock_http_client, Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/0" +
                                         "/queryimages?token=" + expected_token, expected_query_string, _, _)).WillOnce(DoAll(
                                                     SetArgPointee<2>(HttpCode::OK),
                                                     SetArgPointee<3>(nullptr),
                                                     Return(responce_string)));

    data_broker->SetTimeout(100);
    asapo::Error err;
    auto images = data_broker->QueryImages(expected_query_string, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(images.size(), Eq(2));

    ASSERT_THAT(images[0].name, Eq(rec1.name));
    ASSERT_THAT(images[1].name, Eq(rec2.name));
}

TEST_F(ServerDataBrokerTests, QueryImagesUsesCorrectUriWithSubstream) {

    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/" +
                                         expected_substream + "/0" +
                                         "/queryimages?token=" + expected_token, expected_query_string, _, _)).WillOnce(DoAll(
                                                     SetArgPointee<2>(HttpCode::OK),
                                                     SetArgPointee<3>(nullptr),
                                                     Return("[]")));

    data_broker->SetTimeout(100);
    asapo::Error err;
    auto images = data_broker->QueryImages(expected_query_string, expected_substream, &err);

    ASSERT_THAT(err, Eq(nullptr));

}

TEST_F(ServerDataBrokerTests, GetNextDatasetUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/" +
                                        expected_group_id + "/next?token="
                                        + expected_token + "&dataset=true", _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("")));
    asapo::Error err;
    data_broker->GetNextDataset(expected_group_id, &err);
}


TEST_F(ServerDataBrokerTests, GetDataSetReturnsFileInfos) {
    asapo::Error err;
    MockGetBrokerUri();

    auto to_send1 = CreateFI();
    auto json1 = to_send1.Json();
    auto to_send2 = CreateFI();
    to_send2.id = 2;
    auto json2 = to_send2.Json();
    auto to_send3 = CreateFI();
    to_send3.id = 3;
    auto json3 = to_send3.Json();

    auto json = std::string("{") +
                "\"_id\":1," +
                "\"size\":3," +
                "\"images\":[" + json1 + "," + json2 + "," + json3 + "]" +
                "}";

    MockGet(json);

    auto dataset = data_broker->GetNextDataset(expected_group_id, &err);

    ASSERT_THAT(err, Eq(nullptr));

    ASSERT_THAT(dataset.id, Eq(1));
    ASSERT_THAT(dataset.content.size(), Eq(3));
    ASSERT_THAT(dataset.content[0].id, Eq(to_send1.id));
    ASSERT_THAT(dataset.content[1].id, Eq(to_send2.id));
    ASSERT_THAT(dataset.content[2].id, Eq(to_send3.id));
}

TEST_F(ServerDataBrokerTests, GetDataSetReturnsParseError) {
    MockGetBrokerUri();
    MockGet("error_response");

    asapo::Error err;
    auto dataset = data_broker->GetNextDataset(expected_group_id, &err);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kInterruptedTransaction));
    ASSERT_THAT(dataset.content.size(), Eq(0));
    ASSERT_THAT(dataset.id, Eq(0));

}

TEST_F(ServerDataBrokerTests, GetLastDatasetUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/" +
                                        expected_group_id + "/last?token="
                                        + expected_token + "&dataset=true", _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("")));
    asapo::Error err;
    data_broker->GetLastDataset(expected_group_id, &err);
}

TEST_F(ServerDataBrokerTests, GetLastDatasetUsesCorrectUriWithSubstream) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/" +
                                        expected_substream + "/" +
                                        expected_group_id + "/last?token="
                                        + expected_token + "&dataset=true", _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("")));
    asapo::Error err;
    data_broker->GetLastDataset(expected_group_id, expected_substream, &err);
}


TEST_F(ServerDataBrokerTests, GetDatasetByIdUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/default/" +
                                        expected_group_id +
                                        "/" + std::to_string(expected_dataset_id) + "?token="
                                        + expected_token + "&dataset=true", _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("")));
    asapo::Error err;
    data_broker->GetDatasetById(expected_dataset_id, expected_group_id, &err);
}

TEST_F(ServerDataBrokerTests, GetSubstreamListUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_stream + "/0/substreams"
                                        + "?token=" + expected_token, _,
                                        _)).WillOnce(DoAll(
                                                SetArgPointee<1>(HttpCode::OK),
                                                SetArgPointee<2>(nullptr),
                                                Return("{\"substreams\":[\"s1\",\"s2\"]}")));

    asapo::Error err;
    auto substreams = data_broker->GetSubstreamList(&err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(substreams.size(), Eq(2));
    ASSERT_THAT(substreams, testing::ElementsAre("s1", "s2"));

}

void ServerDataBrokerTests::MockBeforeFTS(FileData* data) {
    auto to_send = CreateFI();
    auto json = to_send.Json();
    MockGet(json);

    EXPECT_CALL(mock_netclient, GetData_t(&info,
                                          data)).WillOnce(Return(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()));
}

void ServerDataBrokerTests::ExpectFolderToken() {
    std::string expected_folder_query_string = "{\"Folder\":\"" + expected_path + "\",\"BeamtimeId\":\"" +
                                               expected_beamtime_id
                                               + "\",\"Token\":\"" + expected_token + "\"}";

    EXPECT_CALL(mock_http_client, Post_t(HasSubstr(expected_server_uri + "/authorizer/folder"),
                                         expected_folder_query_string, _, _)).WillOnce(DoAll(
                                                     SetArgPointee<2>(HttpCode::OK),
                                                     SetArgPointee<3>(nullptr),
                                                     Return(expected_folder_token)
                                                 ));

}

ACTION_P(AssignArg3, assign) {
    if (assign) {
        asapo::FileData data = asapo::FileData{new uint8_t[1] };
        data[0] = expected_value;
        *arg3 = std::move(data);
    }
}

void ServerDataBrokerTests::ExpectFileTransfer(const asapo::ConsumerErrorTemplate* p_err_template) {
    EXPECT_CALL(mock_http_client, PostReturnArray_t(HasSubstr(expected_fts_uri + "/transfer"),
                                                    expected_cookie, expected_fts_query_string, _, expected_image_size, _)).WillOnce(DoAll(
                                                            SetArgPointee<5>(HttpCode::OK),
                                                            AssignArg3(p_err_template == nullptr),
                                                            Return(p_err_template == nullptr ? nullptr : p_err_template->Generate().release())
                                                            ));
}

void ServerDataBrokerTests::ExpectRepeatedFileTransfer() {
    EXPECT_CALL(mock_http_client, PostReturnArray_t(HasSubstr(expected_fts_uri + "/transfer"),
                                                    expected_cookie, expected_fts_query_string, _, expected_image_size, _)).
    WillOnce(DoAll(
                 SetArgPointee<5>(HttpCode::Unauthorized),
                 Return(nullptr))).
    WillOnce(DoAll(
                 SetArgPointee<5>(HttpCode::OK),
                 Return(nullptr)
             ));
}

void ServerDataBrokerTests::AssertSingleFileTransfer() {
    asapo::FileData data = asapo::FileData{new uint8_t[1] };
    MockGetBrokerUri();
    MockBeforeFTS(&data);
    ExpectFolderToken();
    MockGetFTSUri();
    ExpectFileTransfer(nullptr);

    fts_data_broker->GetNext(&info, expected_group_id, &data);

    ASSERT_THAT(data[0], Eq(expected_value));
    Mock::VerifyAndClearExpectations(&mock_http_client);
    Mock::VerifyAndClearExpectations(&mock_netclient);
    Mock::VerifyAndClearExpectations(&mock_io);
}


TEST_F(ServerDataBrokerTests, GetImageUsesFileTransferServiceIfCannotReadFromCache) {
    AssertSingleFileTransfer();
}

TEST_F(ServerDataBrokerTests, GetImageReusesTokenAndUri) {
    AssertSingleFileTransfer();

    asapo::FileData data = asapo::FileData{new uint8_t[1] };
    MockBeforeFTS(&data);
    ExpectFileTransfer(nullptr);

    auto err = fts_data_broker->GetNext(&info, expected_group_id, &data);
}

TEST_F(ServerDataBrokerTests, GetImageTriesToGetTokenAgainIfTransferFailed) {
    AssertSingleFileTransfer();

    asapo::FileData data;
    MockBeforeFTS(&data);
    ExpectRepeatedFileTransfer();
    ExpectFolderToken();

    auto err = fts_data_broker->GetNext(&info, expected_group_id, &data);
}

}
