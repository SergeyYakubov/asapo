#include <gmock/gmock.h>
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include <chrono>

#include "asapo/consumer/consumer.h"
#include "asapo/consumer/consumer_error.h"
#include "asapo/io/io.h"
#include "../../../../common/cpp/src/system_io/system_io.h"
#include "../src/consumer_impl.h"
#include "../../../../common/cpp/src/http_client/curl_http_client.h"
#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockHttpClient.h"
#include "asapo/http_client/http_error.h"
#include "mocking.h"
#include "../src/tcp_consumer_client.h"

using asapo::ConsumerFactory;
using asapo::Consumer;
using asapo::ConsumerImpl;
using asapo::IO;
using asapo::MessageMeta;
using asapo::MessageData;
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
using ::testing::ElementsAre;

namespace {

TEST(FolderDataBroker, Constructor) {
    auto consumer =
        std::unique_ptr<ConsumerImpl>{new ConsumerImpl("test", "path", false,
                                                       asapo::SourceCredentials{asapo::SourceType::kProcessed,
                                                                                        "beamtime_id", "", "", "token"})
        };
    ASSERT_THAT(dynamic_cast<asapo::SystemIO*>(consumer->io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::CurlHttpClient*>(consumer->httpclient__.get()), Ne(nullptr));
    ASSERT_THAT(consumer->net_client__.get(), Eq(nullptr));
}

const uint8_t expected_value = 1;

class ConsumerImplTests : public Test {
 public:
  std::unique_ptr<ConsumerImpl> consumer, fts_consumer;
  NiceMock<MockIO> mock_io;
  NiceMock<MockHttpClient> mock_http_client;
  NiceMock<MockNetClient> mock_netclient;
  MessageMeta info;
  std::string expected_server_uri = "test:8400";
  std::string expected_broker_uri = "asapo-broker:5005";
  std::string expected_fts_uri = "asapo-file-transfer:5008";
  std::string expected_token = "token";
  std::string expected_path = "/tmp/beamline/beamtime";
  std::string expected_filename = "filename";
  std::string expected_full_path = std::string("/tmp/beamline/beamtime") + asapo::kPathSeparator + expected_filename;
  std::string expected_group_id = "groupid";
  std::string expected_data_source = "source";
  std::string expected_stream = "stream";
  std::string expected_metadata = "{\"meta\":1}";
  std::string expected_query_string = "bla";
  std::string expected_folder_token = "folder_token";
  std::string expected_beamtime_id = "beamtime_id";
  uint64_t expected_message_size = 100;
  uint64_t expected_dataset_id = 1;
  static const uint64_t expected_buf_id = 123;
  std::string expected_next_stream = "nextstream";
  std::string expected_fts_query_string = "{\"Folder\":\"" + expected_path + "\",\"FileName\":\"" + expected_filename +
      "\"}";
  std::string expected_cookie = "Authorization=Bearer " + expected_folder_token;

  void AssertSingleFileTransfer();
  void SetUp() override {
      consumer = std::unique_ptr<ConsumerImpl>{
          new ConsumerImpl(expected_server_uri,
                           expected_path,
                           true,
                           asapo::SourceCredentials{asapo::SourceType::kProcessed, expected_beamtime_id, "",
                                                        expected_data_source, expected_token})
      };
      fts_consumer = std::unique_ptr<ConsumerImpl>{
          new ConsumerImpl(expected_server_uri,
                           expected_path,
                           false,
                           asapo::SourceCredentials{asapo::SourceType::kProcessed, expected_beamtime_id, "",
                                                        expected_data_source, expected_token})
      };
      consumer->io__ = std::unique_ptr<IO>{&mock_io};
      consumer->httpclient__ = std::unique_ptr<asapo::HttpClient>{&mock_http_client};
      consumer->net_client__ = std::unique_ptr<asapo::NetClient>{&mock_netclient};
      fts_consumer->io__ = std::unique_ptr<IO>{&mock_io};
      fts_consumer->httpclient__ = std::unique_ptr<asapo::HttpClient>{&mock_http_client};
      fts_consumer->net_client__ = std::unique_ptr<asapo::NetClient>{&mock_netclient};

  }
  void TearDown() override {
      consumer->io__.release();
      consumer->httpclient__.release();
      consumer->net_client__.release();
      fts_consumer->io__.release();
      fts_consumer->httpclient__.release();
      fts_consumer->net_client__.release();

  }
  void MockGet(const std::string &response, asapo::HttpCode return_code = HttpCode::OK) {
      EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_broker_uri), _, _)).WillOnce(DoAll(
          SetArgPointee<1>(return_code),
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
      EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/asapo-discovery/" + service), _,
                                          _)).WillOnce(DoAll(
          SetArgPointee<1>(HttpCode::OK),
          SetArgPointee<2>(nullptr),
          Return(result)));
  }

  void MockBeforeFTS(MessageData* data);

  void MockGetFTSUri() {
      MockGetServiceUri("asapo-file-transfer", expected_fts_uri);
  }

  void ExpectFolderToken();
  void ExpectFileTransfer(const asapo::ConsumerErrorTemplate* p_err_template);
  void ExpectRepeatedFileTransfer();
  void ExpectIdList(bool error);
  void ExpectLastAckId(bool empty_response);

  void MockGetBrokerUri() {
      MockGetServiceUri("asapo-broker", expected_broker_uri);
  }
  void MockReadDataFromFile(int times = 1) {
      if (times == 0) {
          EXPECT_CALL(mock_io, GetDataFromFile_t(_, _, _)).Times(0);
          return;
      }

      EXPECT_CALL(mock_io, GetDataFromFile_t(expected_full_path, testing::Pointee(100), _)).Times(times).
          WillRepeatedly(DoAll(SetArgPointee<2>(new asapo::SimpleError{"s"}), testing::Return(nullptr)));
  }
  MessageMeta CreateFI(uint64_t buf_id = expected_buf_id) {
      MessageMeta fi;
      fi.size = expected_message_size;
      fi.id = 1;
      fi.buf_id = buf_id;
      fi.name = expected_filename;
      fi.timestamp = std::chrono::system_clock::now();
      return fi;
  }
};

TEST_F(ConsumerImplTests, GetMessageReturnsErrorOnWrongInput) {
    auto err = consumer->GetNext(nullptr, "", nullptr);
    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kWrongInput));
}

TEST_F(ConsumerImplTests, DefaultStreamIsDetector) {
    consumer->io__.release();
    consumer->httpclient__.release();
    consumer->net_client__.release();
    consumer = std::unique_ptr<ConsumerImpl>{
        new ConsumerImpl(expected_server_uri,
                         expected_path,
                         false,
                         asapo::SourceCredentials{asapo::SourceType::kProcessed, "beamtime_id", "", "",
                                                      expected_token})
    };
    consumer->io__ = std::unique_ptr<IO>{&mock_io};
    consumer->httpclient__ = std::unique_ptr<asapo::HttpClient>{&mock_http_client};
    consumer->net_client__ = std::unique_ptr<asapo::NetClient>{&mock_netclient};

    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client,
                Get_t(expected_broker_uri + "/database/beamtime_id/detector/default/" + expected_group_id
                          +
                              "/next?token="
                          + expected_token, _,
                      _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("")));

    consumer->GetNext(&info, expected_group_id, nullptr);
}

TEST_F(ConsumerImplTests, GetNextUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/"
                                            + expected_group_id + "/next?token="
                                            + expected_token, _,
                                        _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("")));
    consumer->GetNext(&info, expected_group_id, nullptr);
}

TEST_F(ConsumerImplTests, GetNextUsesCorrectUriWithStream) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/" +
                                            expected_stream + "/" + expected_group_id + "/next?token="
                                            + expected_token, _,
                                        _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("")));
    consumer->GetNext(&info, expected_group_id, expected_stream, nullptr);
}

TEST_F(ConsumerImplTests, GetLastUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client,
                Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/0/last?token="
                          + expected_token, _,
                      _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("")));
    consumer->GetLast(&info, nullptr);
}

TEST_F(ConsumerImplTests, GetMessageReturnsEndOfStreamFromHttpClient) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::Conflict),
        SetArgPointee<2>(nullptr),
        Return("{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"\"}")));

    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    auto err_data = static_cast<const asapo::ConsumerErrorData*>(err->GetCustomData());

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kEndOfStream));
    ASSERT_THAT(err_data->id, Eq(1));
    ASSERT_THAT(err_data->id_max, Eq(1));
    ASSERT_THAT(err_data->next_stream, Eq(""));
}

TEST_F(ConsumerImplTests, GetMessageReturnsStreamFinishedFromHttpClient) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::Conflict),
        SetArgPointee<2>(nullptr),
        Return("{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"" + expected_next_stream
                   + "\"}")));

    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    auto err_data = static_cast<const asapo::ConsumerErrorData*>(err->GetCustomData());

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kStreamFinished));
    ASSERT_THAT(err_data->id, Eq(1));
    ASSERT_THAT(err_data->id_max, Eq(1));
    ASSERT_THAT(err_data->next_stream, Eq(expected_next_stream));
}

TEST_F(ConsumerImplTests, GetMessageReturnsNoDataFromHttpClient) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::Conflict),
        SetArgPointee<2>(nullptr),
        Return("{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":2,\"next_stream\":\"""\"}")));

    auto err = consumer->GetNext(&info, expected_group_id, nullptr);
    auto err_data = static_cast<const asapo::ConsumerErrorData*>(err->GetCustomData());

    ASSERT_THAT(err_data->id, Eq(1));
    ASSERT_THAT(err_data->id_max, Eq(2));
    ASSERT_THAT(err_data->next_stream, Eq(""));

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kNoData));
}

TEST_F(ConsumerImplTests, GetMessageReturnsNotAuthorized) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::Unauthorized),
        SetArgPointee<2>(nullptr),
        Return("")));

    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kWrongInput));
}

TEST_F(ConsumerImplTests, GetMessageReturnsWrongResponseFromHttpClient) {

    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::Conflict),
        SetArgPointee<2>(nullptr),
        Return("id")));

    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kInterruptedTransaction));
    ASSERT_THAT(err->Explain(), HasSubstr("malformed"));
}

TEST_F(ConsumerImplTests, GetMessageReturnsIfBrokerAddressNotFound) {
    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/asapo-discovery/asapo-broker"), _,
                                        _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
        SetArgPointee<1>(HttpCode::NotFound),
        SetArgPointee<2>(nullptr),
        Return("")));

    consumer->SetTimeout(100);
    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err->Explain(), AllOf(HasSubstr(expected_server_uri), HasSubstr("unavailable")));
}

TEST_F(ConsumerImplTests, GetMessageReturnsIfBrokerUriEmpty) {
    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/asapo-discovery/asapo-broker"), _,
                                        _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("")));

    consumer->SetTimeout(100);
    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err->Explain(), AllOf(HasSubstr(expected_server_uri), HasSubstr("unavailable")));
}

TEST_F(ConsumerImplTests, GetDoNotCallBrokerUriIfAlreadyFound) {
    MockGetBrokerUri();
    MockGet("error_response");

    consumer->SetTimeout(100);
    consumer->GetNext(&info, expected_group_id, nullptr);
    Mock::VerifyAndClearExpectations(&mock_http_client);

    EXPECT_CALL(mock_http_client,
                Get_t(HasSubstr(expected_server_uri + "/asapo-discovery/asap-broker"), _, _)).Times(0);
    MockGet("error_response");
    consumer->GetNext(&info, expected_group_id, nullptr);
}

TEST_F(ConsumerImplTests, GetBrokerUriAgainAfterConnectionError) {
    MockGetBrokerUri();
    MockGetError();

    consumer->SetTimeout(0);
    consumer->GetNext(&info, expected_group_id, nullptr);
    Mock::VerifyAndClearExpectations(&mock_http_client);

    MockGetBrokerUri();
    MockGet("error_response");
    consumer->GetNext(&info, expected_group_id, nullptr);
}

TEST_F(ConsumerImplTests, GetMessageReturnsEofStreamFromHttpClientUntilTimeout) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
        SetArgPointee<1>(HttpCode::Conflict),
        SetArgPointee<2>(nullptr),
        Return("{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"""\"}")));

    consumer->SetTimeout(300);
    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kEndOfStream));
}

TEST_F(ConsumerImplTests, GetMessageReturnsNoDataAfterTimeoutEvenIfOtherErrorOccured) {
    MockGetBrokerUri();
    consumer->SetTimeout(300);

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::Conflict),
        SetArgPointee<2>(nullptr),
        Return("{\"op\":\"get_record_by_id\",\"id\":" + std::to_string(expected_dataset_id) +
            ",\"id_max\":2,\"next_stream\":\"""\"}")));

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/0/"
                                            + std::to_string(expected_dataset_id) + "?token="
                                            + expected_token, _, _)).Times(AtLeast(1)).WillRepeatedly(DoAll(
        SetArgPointee<1>(HttpCode::NotFound),
        SetArgPointee<2>(nullptr),
        Return("")));

    consumer->SetTimeout(300);
    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kNoData));
}

TEST_F(ConsumerImplTests, GetNextMessageReturnsImmediatelyOnTransferError) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::InternalServerError),
        SetArgPointee<2>(asapo::HttpErrorTemplates::kTransferError.Generate("sss").release()),
        Return("")));

    consumer->SetTimeout(300);
    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kInterruptedTransaction));
    ASSERT_THAT(err->Explain(), HasSubstr("sss"));
}

ACTION(AssignArg2) {
    *arg2 = asapo::HttpErrorTemplates::kConnectionError.Generate().release();
}

TEST_F(ConsumerImplTests, GetNextRetriesIfConnectionHttpClientErrorUntilTimeout) {
    EXPECT_CALL(mock_http_client, Get_t(HasSubstr(expected_server_uri + "/asapo-discovery/asapo-broker"), _,
                                        _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return(expected_broker_uri)));

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).Times(AtLeast(2)).WillRepeatedly(DoAll(
        SetArgPointee<1>(HttpCode::Conflict),
        AssignArg2(),
        Return("")));

    consumer->SetTimeout(300);
    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kUnavailableService));
}

TEST_F(ConsumerImplTests, GetNextMessageReturnsImmediatelyOnFinshedStream) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(HasSubstr("next"), _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::Conflict),
        SetArgPointee<2>(nullptr),
        Return("{\"op\":\"get_record_by_id\",\"id\":2,\"id_max\":2,\"next_stream\":\"next\"}")));

    consumer->SetTimeout(300);
    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kStreamFinished));
}

TEST_F(ConsumerImplTests, GetMessageReturnsMessageMeta) {
    MockGetBrokerUri();

    auto to_send = CreateFI();
    auto json = to_send.Json();

    MockGet(json);

    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(nullptr));

    ASSERT_THAT(info.name, Eq(to_send.name));
    ASSERT_THAT(info.size, Eq(to_send.size));
    ASSERT_THAT(info.id, Eq(to_send.id));
    ASSERT_THAT(info.timestamp, Eq(to_send.timestamp));
}

TEST_F(ConsumerImplTests, GetMessageReturnsParseError) {
    MockGetBrokerUri();
    MockGet("error_response");

    auto err = consumer->GetNext(&info, expected_group_id, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kInterruptedTransaction));
}

TEST_F(ConsumerImplTests, GetMessageReturnsIfNoDataNeeded) {
    MockGetBrokerUri();
    MockGet("error_response");

    EXPECT_CALL(mock_netclient, GetData_t(_, _)).Times(0);
    EXPECT_CALL(mock_io, GetDataFromFile_t(_, _, _)).Times(0);

    consumer->GetNext(&info, expected_group_id, nullptr);
}

TEST_F(ConsumerImplTests, GetMessageTriesToGetDataFromMemoryCache) {
    MockGetBrokerUri();
    auto to_send = CreateFI();
    auto json = to_send.Json();
    MockGet(json);
    MessageData data;

    EXPECT_CALL(mock_netclient, GetData_t(&info, &data)).WillOnce(Return(nullptr));
    MockReadDataFromFile(0);

    consumer->GetNext(&info, expected_group_id, &data);

    ASSERT_THAT(info.buf_id, Eq(expected_buf_id));

}

TEST_F(ConsumerImplTests, GetMessageCallsReadFromFileIfCannotReadFromCache) {
    MockGetBrokerUri();
    auto to_send = CreateFI();
    auto json = to_send.Json();
    MockGet(json);

    MessageData data;

    EXPECT_CALL(mock_netclient, GetData_t(&info,
                                          &data)).WillOnce(Return(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()));
    MockReadDataFromFile();

    consumer->GetNext(&info, expected_group_id, &data);
    ASSERT_THAT(info.buf_id, Eq(0));
}

TEST_F(ConsumerImplTests, GetMessageCallsReadFromFileIfZeroBufId) {
    MockGetBrokerUri();
    auto to_send = CreateFI(0);
    auto json = to_send.Json();
    MockGet(json);

    MessageData data;

    EXPECT_CALL(mock_netclient, GetData_t(_, _)).Times(0);

    MockReadDataFromFile();

    consumer->GetNext(&info, expected_group_id, &data);
}

TEST_F(ConsumerImplTests, GenerateNewGroupIdReturnsErrorCreateGroup) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Post_t(HasSubstr("creategroup"), _, "", _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::BadRequest),
        SetArgPointee<4>(nullptr),
        Return("")));

    consumer->SetTimeout(100);
    asapo::Error err;
    auto groupid = consumer->GenerateNewGroupId(&err);
    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kWrongInput));
    ASSERT_THAT(groupid, Eq(""));
}

TEST_F(ConsumerImplTests, GenerateNewGroupIdReturnsGroupID) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Post_t(expected_broker_uri + "/creategroup?token=" + expected_token, _, "", _,
                                         _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return(expected_group_id)));

    consumer->SetTimeout(100);
    asapo::Error err;
    auto groupid = consumer->GenerateNewGroupId(&err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(groupid, Eq(expected_group_id));
}

TEST_F(ConsumerImplTests, ResetCounterByDefaultUsesCorrectUri) {
    MockGetBrokerUri();
    consumer->SetTimeout(100);

    EXPECT_CALL(mock_http_client,
                Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/" +
                    expected_group_id +
                    "/resetcounter?token=" + expected_token + "&value=0", _, _, _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return("")));
    auto err = consumer->ResetLastReadMarker(expected_group_id);
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ConsumerImplTests, ResetCounterUsesCorrectUri) {
    MockGetBrokerUri();
    consumer->SetTimeout(100);

    EXPECT_CALL(mock_http_client,
                Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/" +
                    expected_group_id +
                    "/resetcounter?token=" + expected_token + "&value=10", _, _, _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return("")));
    auto err = consumer->SetLastReadMarker(expected_group_id, 10);
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ConsumerImplTests, ResetCounterUsesCorrectUriWithStream) {
    MockGetBrokerUri();
    consumer->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/" +
        expected_stream + "/" +
        expected_group_id +
        "/resetcounter?token=" + expected_token + "&value=10", _, _, _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return("")));
    auto err = consumer->SetLastReadMarker(expected_group_id, 10, expected_stream);
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ConsumerImplTests, GetCurrentSizeUsesCorrectUri) {
    MockGetBrokerUri();
    consumer->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source +
        "/default/size?token="
                                            + expected_token, _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("{\"size\":10}")));
    asapo::Error err;
    auto size = consumer->GetCurrentSize(&err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(size, Eq(10));
}

TEST_F(ConsumerImplTests, GetCurrentSizeUsesCorrectUriWithStream) {
    MockGetBrokerUri();
    consumer->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/" +
        expected_stream + "/size?token="
                                            + expected_token, _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("{\"size\":10}")));
    asapo::Error err;
    auto size = consumer->GetCurrentSize(expected_stream, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(size, Eq(10));
}

TEST_F(ConsumerImplTests, GetCurrentSizeErrorOnWrongResponce) {
    MockGetBrokerUri();
    consumer->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source +
        "/default/size?token="
                                            + expected_token, _, _)).WillRepeatedly(DoAll(
        SetArgPointee<1>(HttpCode::Unauthorized),
        SetArgPointee<2>(nullptr),
        Return("")));
    asapo::Error err;
    auto size = consumer->GetCurrentSize(&err);
    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(size, Eq(0));
}

TEST_F(ConsumerImplTests, GetNDataErrorOnWrongParse) {
    MockGetBrokerUri();
    consumer->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source +
        "/default/size?token="
                                            + expected_token, _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("{\"siz\":10}")));
    asapo::Error err;
    auto size = consumer->GetCurrentSize(&err);
    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(size, Eq(0));
}

TEST_F(ConsumerImplTests, GetByIdUsesCorrectUri) {
    MockGetBrokerUri();
    consumer->SetTimeout(100);
    auto to_send = CreateFI();
    auto json = to_send.Json();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/0/"
                                            + std::to_string(
                                                expected_dataset_id) + "?token="
                                            + expected_token, _,
                                        _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return(json)));

    auto err = consumer->GetById(expected_dataset_id, &info, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(info.name, Eq(to_send.name));
}

TEST_F(ConsumerImplTests, GetByIdTimeouts) {
    MockGetBrokerUri();
    consumer->SetTimeout(10);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/0/"
                                            + std::to_string(expected_dataset_id) + "?token="
                                            + expected_token, _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::Conflict),
        SetArgPointee<2>(nullptr),
        Return("")));

    auto err = consumer->GetById(expected_dataset_id, &info, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kNoData));
}

TEST_F(ConsumerImplTests, GetByIdReturnsEndOfStream) {
    MockGetBrokerUri();
    consumer->SetTimeout(10);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/0/"
                                            + std::to_string(expected_dataset_id) + "?token="
                                            + expected_token, _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::Conflict),
        SetArgPointee<2>(nullptr),
        Return("{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"""\"}")));

    auto err = consumer->GetById(expected_dataset_id, &info, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kEndOfStream));
}

TEST_F(ConsumerImplTests, GetByIdReturnsEndOfStreamWhenIdTooLarge) {
    MockGetBrokerUri();
    consumer->SetTimeout(10);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/0/"
                                            + std::to_string(expected_dataset_id) + "?token="
                                            + expected_token, _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::Conflict),
        SetArgPointee<2>(nullptr),
        Return("{\"op\":\"get_record_by_id\",\"id\":100,\"id_max\":1,\"next_stream\":\"""\"}")));

    auto err = consumer->GetById(expected_dataset_id, &info, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kEndOfStream));
}

TEST_F(ConsumerImplTests, GetMetaDataOK) {
    MockGetBrokerUri();
    consumer->SetTimeout(100);

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source +
                                            "/default/0/meta/0?token="
                                            + expected_token, _,
                                        _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return(expected_metadata)));

    asapo::Error err;
    auto res = consumer->GetBeamtimeMeta(&err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(res, Eq(expected_metadata));

}

TEST_F(ConsumerImplTests, QueryMessagesReturnError) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Post_t(HasSubstr("querymessages"), _, expected_query_string, _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::BadRequest),
        SetArgPointee<4>(nullptr),
        Return("error in query")));

    consumer->SetTimeout(1000);
    asapo::Error err;
    auto messages = consumer->QueryMessages(expected_query_string, &err);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kWrongInput));
    ASSERT_THAT(err->Explain(), HasSubstr("query"));
    ASSERT_THAT(messages.size(), Eq(0));
}

TEST_F(ConsumerImplTests, QueryMessagesReturnEmptyResults) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Post_t(HasSubstr("querymessages"), _, expected_query_string, _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return("[]")));

    consumer->SetTimeout(100);
    asapo::Error err;
    auto messages = consumer->QueryMessages(expected_query_string, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(messages.size(), Eq(0));
}

TEST_F(ConsumerImplTests, QueryMessagesWrongResponseArray) {

    MockGetBrokerUri();

    auto rec1 = CreateFI();
    auto rec2 = CreateFI();
    auto json1 = rec1.Json();
    auto json2 = rec2.Json();
    auto responce_string = json1 + "," + json2 + "]"; // no [ at the beginning


    EXPECT_CALL(mock_http_client, Post_t(HasSubstr("querymessages"), _, expected_query_string, _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return(responce_string)));

    consumer->SetTimeout(100);
    asapo::Error err;
    auto messages = consumer->QueryMessages(expected_query_string, &err);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(messages.size(), Eq(0));
    ASSERT_THAT(err->Explain(), HasSubstr("response"));
}

TEST_F(ConsumerImplTests, QueryMessagesWrongResponseRecorsd) {

    MockGetBrokerUri();

    auto responce_string = R"([{"bla":1},{"err":}])";

    EXPECT_CALL(mock_http_client, Post_t(HasSubstr("querymessages"), _, expected_query_string, _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return(responce_string)));

    consumer->SetTimeout(100);
    asapo::Error err;
    auto messages = consumer->QueryMessages(expected_query_string, &err);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(messages.size(), Eq(0));
    ASSERT_THAT(err->Explain(), HasSubstr("response"));
}

TEST_F(ConsumerImplTests, QueryMessagesReturnRecords) {

    MockGetBrokerUri();

    auto rec1 = CreateFI();
    auto rec2 = CreateFI();
    rec2.name = "ttt";
    auto json1 = rec1.Json();
    auto json2 = rec2.Json();
    auto responce_string = "[" + json1 + "," + json2 + "]";

    EXPECT_CALL(mock_http_client,
                Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/0" +
                    "/querymessages?token=" + expected_token, _, expected_query_string, _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return(responce_string)));

    consumer->SetTimeout(100);
    asapo::Error err;
    auto messages = consumer->QueryMessages(expected_query_string, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(messages.size(), Eq(2));

    ASSERT_THAT(messages[0].name, Eq(rec1.name));
    ASSERT_THAT(messages[1].name, Eq(rec2.name));
}

TEST_F(ConsumerImplTests, QueryMessagesUsesCorrectUriWithStream) {

    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/" +
        expected_stream + "/0" +
        "/querymessages?token=" + expected_token, _, expected_query_string, _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return("[]")));

    consumer->SetTimeout(100);
    asapo::Error err;
    auto messages = consumer->QueryMessages(expected_query_string, expected_stream, &err);

    ASSERT_THAT(err, Eq(nullptr));

}

TEST_F(ConsumerImplTests, GetNextDatasetUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/" +
                                            expected_group_id + "/next?token="
                                            + expected_token + "&dataset=true&minsize=0", _,
                                        _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("")));
    asapo::Error err;
    consumer->GetNextDataset(expected_group_id, 0, &err);
}

TEST_F(ConsumerImplTests, GetDataSetReturnsMessageMetas) {
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
        "\"messages\":[" + json1 + "," + json2 + "," + json3 + "]" +
        "}";

    MockGet(json);

    auto dataset = consumer->GetNextDataset(expected_group_id, 0, &err);

    ASSERT_THAT(err, Eq(nullptr));

    ASSERT_THAT(dataset.id, Eq(1));
    ASSERT_THAT(dataset.content.size(), Eq(3));
    ASSERT_THAT(dataset.content[0].id, Eq(to_send1.id));
    ASSERT_THAT(dataset.content[1].id, Eq(to_send2.id));
    ASSERT_THAT(dataset.content[2].id, Eq(to_send3.id));
}

TEST_F(ConsumerImplTests, GetDataSetReturnsPartialMessageMetas) {
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
        "\"messages\":[" + json1 + "," + json2 + "]" +
        "}";

    MockGet(json, asapo::HttpCode::PartialContent);

    auto dataset = consumer->GetNextDataset(expected_group_id, 0, &err);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kPartialData));

    auto err_data = static_cast<const asapo::PartialErrorData*>(err->GetCustomData());
    ASSERT_THAT(err_data->expected_size, Eq(3));
    ASSERT_THAT(err_data->id, Eq(1));

    ASSERT_THAT(dataset.id, Eq(1));
    ASSERT_THAT(dataset.content.size(), Eq(2));
    ASSERT_THAT(dataset.content[0].id, Eq(to_send1.id));
    ASSERT_THAT(dataset.content[1].id, Eq(to_send2.id));
}

TEST_F(ConsumerImplTests, GetDataSetByIdReturnsPartialMessageMetas) {
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
        "\"messages\":[" + json1 + "," + json2 + "]" +
        "}";

    MockGet(json, asapo::HttpCode::PartialContent);

    auto dataset = consumer->GetDatasetById(1, 0, &err);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kPartialData));
    auto err_data = static_cast<const asapo::PartialErrorData*>(err->GetCustomData());
    ASSERT_THAT(err_data->expected_size, Eq(3));
    ASSERT_THAT(err_data->id, Eq(1));

    ASSERT_THAT(dataset.id, Eq(1));
    ASSERT_THAT(dataset.content.size(), Eq(2));
    ASSERT_THAT(dataset.content[0].id, Eq(to_send1.id));
    ASSERT_THAT(dataset.content[1].id, Eq(to_send2.id));
}

TEST_F(ConsumerImplTests, GetDataSetReturnsParseError) {
    MockGetBrokerUri();
    MockGet("error_response");

    asapo::Error err;
    auto dataset = consumer->GetNextDataset(expected_group_id, 0, &err);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kInterruptedTransaction));
    ASSERT_THAT(dataset.content.size(), Eq(0));
    ASSERT_THAT(dataset.id, Eq(0));

}

TEST_F(ConsumerImplTests, GetLastDatasetUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client,
                Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/0/last?token="
                          + expected_token + "&dataset=true&minsize=2", _,
                      _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("")));
    asapo::Error err;
    consumer->GetLastDataset(2, &err);
}

TEST_F(ConsumerImplTests, GetLastDatasetUsesCorrectUriWithStream) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/" +
                                            expected_stream + "/0/last?token="
                                            + expected_token + "&dataset=true&minsize=1", _,
                                        _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("")));
    asapo::Error err;
    consumer->GetLastDataset(expected_stream, 1, &err);
}

TEST_F(ConsumerImplTests, GetDatasetByIdUsesCorrectUri) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/0/"
                                            + std::to_string(expected_dataset_id) + "?token="
                                            + expected_token + "&dataset=true" + "&minsize=0", _,
                                        _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("")));
    asapo::Error err;
    consumer->GetDatasetById(expected_dataset_id, 0, &err);
}

TEST_F(ConsumerImplTests, GetStreamListUsesCorrectUri) {
    MockGetBrokerUri();
    std::string return_streams =
        R"({"streams":[{"lastId":123,"name":"test","timestampCreated":1000000},{"name":"test1","timestampCreated":2000000}]})";
    EXPECT_CALL(mock_http_client,
                Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/0/streams"
                          + "?token=" + expected_token + "&from=stream_from", _,
                      _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return(return_streams)));

    asapo::Error err;
    auto streams = consumer->GetStreamList("stream_from", &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(streams.size(), Eq(2));
    ASSERT_THAT(streams.size(), 2);
    ASSERT_THAT(streams[0].Json(false), R"({"name":"test","timestampCreated":1000000})");
    ASSERT_THAT(streams[1].Json(false), R"({"name":"test1","timestampCreated":2000000})");
}

TEST_F(ConsumerImplTests, GetStreamListUsesCorrectUriWithoutFrom) {
    MockGetBrokerUri();
    EXPECT_CALL(mock_http_client,
                Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/0/streams"
                          + "?token=" + expected_token, _,
                      _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("")));;

    asapo::Error err;
    auto streams = consumer->GetStreamList("", &err);
}

void ConsumerImplTests::MockBeforeFTS(MessageData* data) {
    auto to_send = CreateFI();
    auto json = to_send.Json();
    MockGet(json);

    EXPECT_CALL(mock_netclient, GetData_t(&info,
                                          data)).WillOnce(Return(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()));
}

void ConsumerImplTests::ExpectFolderToken() {
    std::string expected_folder_query_string = "{\"Folder\":\"" + expected_path + "\",\"BeamtimeId\":\"" +
        expected_beamtime_id
        + "\",\"Token\":\"" + expected_token + "\"}";

    EXPECT_CALL(mock_http_client, Post_t(HasSubstr(expected_server_uri + "/asapo-authorizer/folder"), _,
                                         expected_folder_query_string, _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return(expected_folder_token)
    ));

}

ACTION_P(AssignArg3, assign) {
    if (assign) {
        asapo::MessageData data = asapo::MessageData{new uint8_t[1]};
        data[0] = expected_value;
        *arg3 = std::move(data);
    }
}

void ConsumerImplTests::ExpectFileTransfer(const asapo::ConsumerErrorTemplate* p_err_template) {
    EXPECT_CALL(mock_http_client, PostReturnArray_t(HasSubstr(expected_fts_uri + "/transfer"),
                                                    expected_cookie,
                                                    expected_fts_query_string,
                                                    _,
                                                    expected_message_size,
                                                    _)).WillOnce(DoAll(
        SetArgPointee<5>(HttpCode::OK),
        AssignArg3(p_err_template == nullptr),
        Return(p_err_template == nullptr ? nullptr : p_err_template->Generate().release())
    ));
}

void ConsumerImplTests::ExpectRepeatedFileTransfer() {
    EXPECT_CALL(mock_http_client, PostReturnArray_t(HasSubstr(expected_fts_uri + "/transfer"),
                                                    expected_cookie,
                                                    expected_fts_query_string,
                                                    _,
                                                    expected_message_size,
                                                    _)).
        WillOnce(DoAll(
        SetArgPointee<5>(HttpCode::Unauthorized),
        Return(nullptr))).
        WillOnce(DoAll(
        SetArgPointee<5>(HttpCode::OK),
        Return(nullptr)
    ));
}

void ConsumerImplTests::AssertSingleFileTransfer() {
    asapo::MessageData data = asapo::MessageData{new uint8_t[1]};
    MockGetBrokerUri();
    MockBeforeFTS(&data);
    ExpectFolderToken();
    MockGetFTSUri();
    ExpectFileTransfer(nullptr);

    fts_consumer->GetNext(&info, expected_group_id, &data);

    ASSERT_THAT(data[0], Eq(expected_value));
    Mock::VerifyAndClearExpectations(&mock_http_client);
    Mock::VerifyAndClearExpectations(&mock_netclient);
    Mock::VerifyAndClearExpectations(&mock_io);
}

TEST_F(ConsumerImplTests, GetMessageUsesFileTransferServiceIfCannotReadFromCache) {
    AssertSingleFileTransfer();
}

TEST_F(ConsumerImplTests, FileTransferReadsFileSize) {
    AssertSingleFileTransfer();
    EXPECT_CALL(mock_http_client, Post_t(HasSubstr("sizeonly=true"),
                                         expected_cookie, expected_fts_query_string, _, _)).WillOnce(DoAll(

        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return("{\"file_size\":5}")
    ));

    EXPECT_CALL(mock_http_client, PostReturnArray_t(HasSubstr(expected_fts_uri + "/transfer"),
                                                    expected_cookie,
                                                    expected_fts_query_string,
                                                    _,
                                                    5,
                                                    _)).WillOnce(DoAll(
        SetArgPointee<5>(HttpCode::OK),
        AssignArg3(nullptr),
        Return(nullptr)
    ));

    MessageData data;
    info.size = 0;
    info.buf_id = 0;
    auto err = fts_consumer->RetrieveData(&info, &data);
}

TEST_F(ConsumerImplTests, GetMessageReusesTokenAndUri) {
    AssertSingleFileTransfer();

    asapo::MessageData data = asapo::MessageData{new uint8_t[1]};
    MockBeforeFTS(&data);
    ExpectFileTransfer(nullptr);

    auto err = fts_consumer->GetNext(&info, expected_group_id, &data);
}

TEST_F(ConsumerImplTests, GetMessageTriesToGetTokenAgainIfTransferFailed) {
    AssertSingleFileTransfer();

    asapo::MessageData data;
    MockBeforeFTS(&data);
    ExpectRepeatedFileTransfer();
    ExpectFolderToken();

    auto err = fts_consumer->GetNext(&info, expected_group_id, &data);
}

TEST_F(ConsumerImplTests, AcknowledgeUsesCorrectUri) {
    MockGetBrokerUri();
    auto expected_acknowledge_command = "{\"Op\":\"ackmessage\"}";
    EXPECT_CALL(mock_http_client, Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/" +
        expected_stream + "/" +
        expected_group_id
                                             + "/" + std::to_string(expected_dataset_id) + "?token="
                                             + expected_token, _, expected_acknowledge_command, _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return("")));

    auto err = consumer->Acknowledge(expected_group_id, expected_dataset_id, expected_stream);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ConsumerImplTests, AcknowledgeUsesCorrectUriWithDefaultStream) {
    MockGetBrokerUri();
    auto expected_acknowledge_command = "{\"Op\":\"ackmessage\"}";
    EXPECT_CALL(mock_http_client,
                Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/" +
                    expected_group_id
                           + "/" + std::to_string(expected_dataset_id) + "?token="
                           + expected_token, _, expected_acknowledge_command, _, _)).WillOnce(DoAll(
        SetArgPointee<3>(HttpCode::OK),
        SetArgPointee<4>(nullptr),
        Return("")));

    auto err = consumer->Acknowledge(expected_group_id, expected_dataset_id);

    ASSERT_THAT(err, Eq(nullptr));
}

void ConsumerImplTests::ExpectIdList(bool error) {
    MockGetBrokerUri();
    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/" +
        expected_stream + "/" +
        expected_group_id + "/nacks?token=" + expected_token + "&from=1&to=0", _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return(error ? "" : "{\"unacknowledged\":[1,2,3]}")));
}

TEST_F(ConsumerImplTests, GetUnAcknowledgedListReturnsIds) {
    ExpectIdList(false);
    asapo::Error err;
    auto list = consumer->GetUnacknowledgedMessages(expected_group_id, expected_stream, 1, 0, &err);

    ASSERT_THAT(list, ElementsAre(1, 2, 3));
    ASSERT_THAT(err, Eq(nullptr));
}

void ConsumerImplTests::ExpectLastAckId(bool empty_response) {
    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/" +
        expected_stream + "/" +
        expected_group_id + "/lastack?token=" + expected_token, _, _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return(empty_response ? "{\"lastAckId\":0}" : "{\"lastAckId\":1}")));
}

TEST_F(ConsumerImplTests, GetLastAcknowledgeUsesOk) {
    MockGetBrokerUri();
    ExpectLastAckId(false);

    asapo::Error err;
    auto ind = consumer->GetLastAcknowledgedMessage(expected_group_id, expected_stream, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(ind, Eq(1));
}

TEST_F(ConsumerImplTests, GetLastAcknowledgeReturnsNoData) {
    MockGetBrokerUri();
    ExpectLastAckId(true);

    asapo::Error err;
    auto ind = consumer->GetLastAcknowledgedMessage(expected_group_id, expected_stream, &err);
    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kNoData));
    ASSERT_THAT(ind, Eq(0));
}

TEST_F(ConsumerImplTests, GetByIdErrorsForId0) {

    auto err = consumer->GetById(0, &info, nullptr);

    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kWrongInput));
}

TEST_F(ConsumerImplTests, ResendNacks) {
    MockGetBrokerUri();

    EXPECT_CALL(mock_http_client, Get_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/default/"
                                            + expected_group_id + "/next?token="
                                            + expected_token + "&resend_nacks=true&delay_ms=10000&resend_attempts=3", _,
                                        _)).WillOnce(DoAll(
        SetArgPointee<1>(HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return("")));

    consumer->SetResendNacs(true, 10000, 3);
    consumer->GetNext(&info, expected_group_id, nullptr);
}

TEST_F(ConsumerImplTests, NegativeAcknowledgeUsesCorrectUri) {
    MockGetBrokerUri();
    auto expected_neg_acknowledge_command = R"({"Op":"negackmessage","Params":{"DelayMs":10000}})";
    EXPECT_CALL(mock_http_client, Post_t(expected_broker_uri + "/database/beamtime_id/" + expected_data_source + "/" +
        expected_stream + "/" +
        expected_group_id
                                             + "/" + std::to_string(expected_dataset_id) + "?token="
                                             + expected_token, _, expected_neg_acknowledge_command, _, _)).WillOnce(
        DoAll(
            SetArgPointee<3>(HttpCode::OK),
            SetArgPointee<4>(nullptr),
            Return("")));

    auto err = consumer->NegativeAcknowledge(expected_group_id, expected_dataset_id, 10000, expected_stream);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ConsumerImplTests, CanInterruptOperation) {
    EXPECT_CALL(mock_http_client, Get_t(_, _, _)).Times(AtLeast(1)).WillRepeatedly(DoAll(
        SetArgPointee<1>(HttpCode::NotFound),
        SetArgPointee<2>(nullptr),
        Return("")));

    auto start = std::chrono::system_clock::now();
    asapo::Error err;
    auto exec = [this,&err]() {
      consumer->SetTimeout(10000);
      err = consumer->GetNext(&info, "", nullptr);
    };
    auto thread = std::thread(exec);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    consumer->InterruptCurrentOperation();

    thread.join();

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
    ASSERT_THAT(elapsed_ms, testing::Lt(1000));
    ASSERT_THAT(err, Eq(asapo::ConsumerErrorTemplates::kInterruptedTransaction));

}

}
