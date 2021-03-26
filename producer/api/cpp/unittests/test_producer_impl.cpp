#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockLogger.h"
#include "asapo/common/error.h"
#include "asapo/producer/common.h"
#include "../src/producer_impl.h"
#include "asapo/producer/producer_error.h"

#include "../src/request_handler_tcp.h"
#include "asapo/request/request_pool_error.h"
#include "asapo/unittests/MockHttpClient.h"

#include "mocking.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::InSequence;
using ::testing::HasSubstr;
using testing::SetArgPointee;


using asapo::RequestPool;
using asapo::ProducerRequest;
using asapo::MockHttpClient;


MATCHER_P10(M_CheckSendRequest, op_code, source_credentials, metadata, file_id, file_size, message, stream,
            ingest_mode,
            dataset_id,
            dataset_size,
            "Checks if a valid GenericRequestHeader was Send") {
    auto request = static_cast<ProducerRequest*>(arg);
    return ((asapo::GenericRequestHeader) (arg->header)).op_code == op_code
        && ((asapo::GenericRequestHeader) (arg->header)).data_id == file_id
        && ((asapo::GenericRequestHeader) (arg->header)).data_size == uint64_t(file_size)
        && request->manage_data_memory == true
        && request->source_credentials == source_credentials
        && request->metadata == metadata
        && (op_code == asapo::kOpcodeTransferDatasetData ? ((asapo::GenericRequestHeader) (arg->header)).custom_data[1]
            == uint64_t(dataset_id) : true)
        && (op_code == asapo::kOpcodeTransferDatasetData ? ((asapo::GenericRequestHeader) (arg->header)).custom_data[2]
            == uint64_t(dataset_size) : true)
        && ((asapo::GenericRequestHeader) (arg->header)).custom_data[asapo::kPosIngestMode] == uint64_t(ingest_mode)
        && strcmp(((asapo::GenericRequestHeader) (arg->header)).message, message) == 0
        && strcmp(((asapo::GenericRequestHeader) (arg->header)).stream, stream) == 0;
}

TEST(ProducerImpl, Constructor) {
    asapo::ProducerImpl producer{"", 4, 3600000, asapo::RequestHandlerType::kTcp};
    ASSERT_THAT(dynamic_cast<asapo::AbstractLogger*>(producer.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::RequestPool*>(producer.request_pool__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::HttpClient*>(producer.httpclient__.get()), Ne(nullptr));

}

class ProducerImplTests : public testing::Test {
 public:
  testing::NiceMock<MockDiscoveryService> service;
  asapo::ProducerRequestHandlerFactory factory{&service};
  testing::NiceMock<asapo::MockLogger> mock_logger;
  testing::NiceMock<MockRequestPull> mock_pull{&factory, &mock_logger};
  std::string expected_server_uri = "test:8400";
  asapo::ProducerImpl producer{expected_server_uri, 1, 3600000, asapo::RequestHandlerType::kTcp};
  uint64_t expected_size = 100;
  uint64_t expected_id = 10;
  uint64_t expected_dataset_id = 100;
  uint64_t expected_dataset_size = 4;
  uint64_t expected_ingest_mode = asapo::IngestModeFlags::kTransferMetaDataOnly;

  char expected_name[asapo::kMaxMessageSize] = "test_name";
  char expected_stream[asapo::kMaxMessageSize] = "test_stream";
  std::string expected_next_stream = "next_stream";

  asapo::SourceCredentials expected_credentials{asapo::SourceType::kRaw, "beamtime_id", "beamline", "subname", "token"
  };
  asapo::SourceCredentials expected_default_credentials{
      asapo::SourceType::kProcessed, "beamtime_id", "", "", "token"
  };

  std::string expected_credentials_str = "raw%beamtime_id%beamline%subname%token";
  std::string expected_default_credentials_str = "processed%beamtime_id%auto%detector%token";

  std::string expected_metadata = "meta";
  std::string expected_fullpath = "filename";
  bool expected_managed_memory = true;
  bool expected_unmanaged_memory = false;

  MockHttpClient* mock_http_client;

  void SetUp() override {
      producer.log__ = &mock_logger;
      producer.request_pool__ = std::unique_ptr<RequestPool>{&mock_pull};
      mock_http_client = new MockHttpClient;
      producer.httpclient__.reset(mock_http_client);

  }
  void TearDown() override {
      producer.request_pool__.release();
  }
};

TEST_F(ProducerImplTests, SendReturnsError) {
    EXPECT_CALL(mock_pull, AddRequest_t(_, false)).WillOnce(Return(
        asapo::IOErrorTemplates::kNoSpaceLeft.Generate().release()));
    asapo::MessageHeader message_header{1, 1, "test"};
    auto err = producer.Send(message_header, nullptr, expected_ingest_mode, "default", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kRequestPoolIsFull));
}

TEST_F(ProducerImplTests, ErrorIfFileNameTooLong) {
    asapo::MessageData data = asapo::MessageData{new uint8_t[100]};
    data[34]=12;
    std::string long_string(asapo::kMaxMessageSize + 100, 'a');
    asapo::MessageHeader message_header{1, 1, long_string};
    auto err = producer.Send(message_header, std::move(data), expected_ingest_mode, "default", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
    auto err_data = static_cast<asapo::OriginalData*>(err->GetCustomData());
    ASSERT_THAT(err_data, Ne(nullptr));
    ASSERT_THAT(err_data->data[34], Eq(12));
}

TEST_F(ProducerImplTests, ErrorIfStreamEmpty) {
    asapo::MessageHeader message_header{1, 100, expected_fullpath};
    auto err = producer.Send(message_header, nullptr, expected_ingest_mode, "", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}


TEST_F(ProducerImplTests, ErrorIfFileEmpty) {
    std::string long_string(asapo::kMaxMessageSize + 100, 'a');
    asapo::MessageHeader message_header{1, 1, ""};
    auto err = producer.Send(message_header, nullptr, expected_ingest_mode, "default", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, ErrorIfDatasetSizeNotDefined) {
    EXPECT_CALL(mock_logger, Error(testing::HasSubstr("dataset dimensions")));
    asapo::MessageHeader message_header{1, 1000, "test", "", 1};
    auto err = producer.Send(message_header, nullptr, expected_ingest_mode, "default", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, ErrorIfZeroDataSize) {
    asapo::MessageData data = asapo::MessageData{new uint8_t[100]};
    asapo::MessageHeader message_header{1, 0, expected_fullpath};
    auto err = producer.Send(message_header, std::move(data), asapo::kDefaultIngestMode, "default", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
    auto err_data = static_cast<asapo::OriginalData*>(err->GetCustomData());
    ASSERT_THAT(err_data, Ne(nullptr));
}

TEST_F(ProducerImplTests, ErrorIfNoData) {
    asapo::MessageHeader message_header{1, 100, expected_fullpath};
    auto err = producer.Send(message_header, nullptr, asapo::kDefaultIngestMode, "default", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, ErrorIfNoDataSend_) {
    asapo::MessageHeader message_header{1, 100, expected_fullpath};
    auto err = producer.Send__(message_header, nullptr, asapo::kDefaultIngestMode, expected_stream, nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, ErrorIfSendingDataWithZeroId) {
    asapo::MessageHeader message_header{0, 100, expected_fullpath};
    auto err = producer.Send(message_header, nullptr, asapo::kTransferMetaDataOnly, "default", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, OkIfNoDataWithTransferMetadataOnlyMode) {
    asapo::MessageHeader message_header{1, 100, expected_fullpath};
    auto err = producer.Send(message_header, nullptr, asapo::kTransferMetaDataOnly, "default", nullptr);
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OkIfZeroSizeWithTransferMetadataOnlyMode) {
    asapo::MessageData data = asapo::MessageData{new uint8_t[100]};
    asapo::MessageHeader message_header{1, 0, expected_fullpath};
    auto err = producer.Send(message_header, std::move(data), asapo::kTransferMetaDataOnly, "default", nullptr);
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKSendingSendRequestWithStream) {
    producer.SetCredentials(expected_credentials);

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendRequest(asapo::kOpcodeTransferData,
                                                               expected_credentials_str,
                                                               expected_metadata,
                                                               expected_id,
                                                               expected_size,
                                                               expected_name,
                                                               expected_stream,
                                                               expected_ingest_mode,
                                                               0,
                                                               0
    ), false)).WillOnce(Return(
        nullptr));

    asapo::MessageHeader message_header{expected_id, expected_size, expected_name, expected_metadata};
    auto err = producer.Send(message_header, nullptr, expected_ingest_mode, expected_stream, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKSendingStreamFinish) {
    producer.SetCredentials(expected_credentials);

    std::string next_stream_meta = std::string("{\"next_stream\":") + "\"" + expected_next_stream + "\"}";

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendRequest(asapo::kOpcodeTransferData,
                                                               expected_credentials_str,
                                                               next_stream_meta.c_str(),
                                                               expected_id + 1,
                                                               0,
                                                               asapo::kFinishStreamKeyword.c_str(),
                                                               expected_stream,
                                                               asapo::IngestModeFlags::kTransferMetaDataOnly,
                                                               0,
                                                               0
    ), false)).WillOnce(Return(
        nullptr));

    auto err = producer.SendStreamFinishedFlag(expected_stream, expected_id, expected_next_stream, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, ErrorSendingStreamFinishWithemptyStream) {
    producer.SetCredentials(expected_credentials);
    auto err = producer.SendStreamFinishedFlag("", expected_id, expected_next_stream, nullptr);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, OKSendingStreamFinishWithNoNextStream) {
    producer.SetCredentials(expected_credentials);

    std::string
        next_stream_meta = std::string("{\"next_stream\":") + "\"" + asapo::kNoNextStreamKeyword
        + "\"}";

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendRequest(asapo::kOpcodeTransferData,
                                                               expected_credentials_str,
                                                               next_stream_meta.c_str(),
                                                               expected_id + 1,
                                                               0,
                                                               asapo::kFinishStreamKeyword.c_str(),
                                                               expected_stream,
                                                               asapo::IngestModeFlags::kTransferMetaDataOnly,
                                                               0,
                                                               0
    ), false)).WillOnce(Return(
        nullptr));

    auto err = producer.SendStreamFinishedFlag(expected_stream, expected_id, "", nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKSendingSendDatasetDataRequest) {
    producer.SetCredentials(expected_credentials);
    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendRequest(asapo::kOpcodeTransferDatasetData,
                                                               expected_credentials_str,
                                                               expected_metadata,
                                                               expected_id,
                                                               expected_size,
                                                               expected_name,
                                                               expected_stream,
                                                               expected_ingest_mode,
                                                               expected_dataset_id,
                                                               expected_dataset_size), false)).WillOnce(
        Return(
            nullptr));

    asapo::MessageHeader message_header
        {expected_id, expected_size, expected_name, expected_metadata, expected_dataset_id, expected_dataset_size};
    auto err = producer.Send(message_header, nullptr, expected_ingest_mode, expected_stream, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKAddingSendMetaDataRequest) {
    expected_id = 0;
    expected_metadata = "{\"meta\":10}";
    expected_size = expected_metadata.size();
    expected_ingest_mode = asapo::IngestModeFlags::kTransferData | asapo::IngestModeFlags::kStoreInDatabase ;

    producer.SetCredentials(expected_credentials);
    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendRequest(asapo::kOpcodeTransferMetaData,
                                                               expected_credentials_str,
                                                               "",
                                                               expected_id,
                                                               expected_size,
                                                               "beamtime_global.meta",
                                                               "",
                                                               expected_ingest_mode,
                                                               10,
                                                               10), false)).WillOnce(Return(
        nullptr));

    auto err = producer.SendMetadata(expected_metadata, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, ErrorSendingEmptyFileName) {
    producer.SetCredentials(expected_credentials);

    EXPECT_CALL(mock_pull, AddRequest_t(_, _)).Times(0);

    asapo::MessageHeader message_header{expected_id, 0, expected_name};
    auto err = producer.SendFile(message_header, "", expected_ingest_mode, expected_stream, nullptr);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));

}

TEST_F(ProducerImplTests, ErrorSendingEmptyRelativeFileName) {
    producer.SetCredentials(expected_credentials);

    EXPECT_CALL(mock_pull, AddRequest_t(_, _)).Times(0);

    asapo::MessageHeader message_header{expected_id, 0, ""};
    auto err = producer.SendFile(message_header, expected_fullpath, expected_ingest_mode, expected_stream, nullptr);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));

}

TEST_F(ProducerImplTests, ErrorSendingFileToEmptyStream) {
    producer.SetCredentials(expected_credentials);

    EXPECT_CALL(mock_pull, AddRequest_t(_, _)).Times(0);

    asapo::MessageHeader message_header{expected_id, 0, expected_name};
    auto err = producer.SendFile(message_header, expected_fullpath, expected_ingest_mode, "", nullptr);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));

}

TEST_F(ProducerImplTests, OKSendingSendFileRequestWithStream) {
    producer.SetCredentials(expected_credentials);

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendRequest(asapo::kOpcodeTransferData,
                                                               expected_credentials_str,
                                                               "",
                                                               expected_id,
                                                               0,
                                                               expected_name,
                                                               expected_stream,
                                                               expected_ingest_mode,
                                                               0,
                                                               0), false)).WillOnce(Return(
        nullptr));

    asapo::MessageHeader message_header{expected_id, 0, expected_name};
    auto err =
        producer.SendFile(message_header, expected_fullpath, expected_ingest_mode, expected_stream, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, ErrorSettingBeamtime) {
    std::string long_str(asapo::kMaxMessageSize * 10, 'a');
    expected_credentials = asapo::SourceCredentials{asapo::SourceType::kRaw, long_str, "", "", ""};
    EXPECT_CALL(mock_logger, Error(testing::HasSubstr("too long")));

    auto err = producer.SetCredentials(expected_credentials);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, ErrorSettingSecondTime) {
    EXPECT_CALL(mock_logger, Error(testing::HasSubstr("already")));

    producer.SetCredentials(asapo::SourceCredentials{asapo::SourceType::kRaw, "1", "", "2", "3"});
    auto err = producer.SetCredentials(asapo::SourceCredentials{asapo::SourceType::kRaw, "4", "", "5", "6"});

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, ErrorSendingWrongIngestMode) {
    producer.SetCredentials(expected_credentials);
    asapo::MessageHeader message_header{expected_id, 0, expected_name};
    uint64_t ingest_modes[] = {0, asapo::IngestModeFlags::kTransferMetaDataOnly | asapo::IngestModeFlags::kTransferData,
                               asapo::IngestModeFlags::kTransferData,
                               asapo::IngestModeFlags::kStoreInDatabase,
                               asapo::IngestModeFlags::kStoreInFilesystem,
                               asapo::IngestModeFlags::kStoreInDatabase | asapo::IngestModeFlags::kStoreInFilesystem,
                               asapo::IngestModeFlags::kTransferMetaDataOnly
                                   | asapo::IngestModeFlags::kStoreInFilesystem};

    EXPECT_CALL(mock_pull, AddRequest_t(_, _)).Times(0);

    for (auto ingest_mode : ingest_modes) {
        auto err = producer.SendFile(message_header, expected_fullpath, ingest_mode, expected_stream, nullptr);
        ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
    }

}

TEST_F(ProducerImplTests, GetQueueSize) {
    EXPECT_CALL(mock_pull, NRequestsInPool()).WillOnce(Return(10));

    auto size = producer.GetRequestsQueueSize();
    ASSERT_THAT(size, Eq(10));
}

TEST_F(ProducerImplTests, GetQueueVolume) {
    EXPECT_CALL(mock_pull, UsedMemoryInPool()).WillOnce(Return(10000000));

    auto vol = producer.GetRequestsQueueVolumeMb();

    ASSERT_THAT(vol, Eq(10));
}

MATCHER_P(M_CheckLimits, limits,"Checks if a valid limits were used") {
    return arg.max_requests == limits.max_requests && arg.max_memory_mb == limits.max_memory_mb;
};

TEST_F(ProducerImplTests, SetLimits) {
    EXPECT_CALL(mock_pull, SetLimits(M_CheckLimits(asapo::RequestPoolLimits{10,20})));

    producer.SetRequestsQueueLimits(10,20);
}

TEST_F(ProducerImplTests, WaitRequestsFinished) {
    EXPECT_CALL(mock_pull, WaitRequestsFinished_t(_)).WillOnce(Return(
        asapo::IOErrorTemplates::kTimeout.Generate().release()));

    auto err = producer.WaitRequestsFinished(100);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kTimeout));
}

MATCHER_P3(M_CheckGetStreamInfoRequest, op_code, source_credentials, stream,
           "Checks if a valid GenericRequestHeader was Send") {
    auto request = static_cast<ProducerRequest*>(arg);
    return ((asapo::GenericRequestHeader) (arg->header)).op_code == op_code
        && request->source_credentials == source_credentials
        && strcmp(((asapo::GenericRequestHeader) (arg->header)).stream, stream) == 0;
}

TEST_F(ProducerImplTests, GetStreamInfoMakesCorerctRequest) {
    producer.SetCredentials(expected_credentials);
    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckGetStreamInfoRequest(asapo::kOpcodeStreamInfo,
                                                                       expected_credentials_str,
                                                                       expected_stream), true)).WillOnce(
        Return(nullptr));

    asapo::Error err;
    producer.GetStreamInfo(expected_stream, 1000, &err);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kTimeout));
}

TEST_F(ProducerImplTests, GetStreamInfoErrorOnEmptyStream) {
    producer.SetCredentials(expected_credentials);
    asapo::Error err;
    producer.GetStreamInfo("", 1000, &err);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST(GetStreamInfoTest, GetStreamInfoTimeout) {
    asapo::ProducerImpl producer1{"", 1, 10000, asapo::RequestHandlerType::kTcp};
    asapo::Error err;
    auto sinfo = producer1.GetStreamInfo("stream", 5000, &err);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kTimeout));
    ASSERT_THAT(err->Explain(), HasSubstr("opcode: 4"));
}

TEST_F(ProducerImplTests, GetLastStreamMakesCorerctRequest) {
    producer.SetCredentials(expected_credentials);
    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckGetStreamInfoRequest(asapo::kOpcodeLastStream,
                                                                       expected_credentials_str,
                                                                       ""), true)).WillOnce(
        Return(nullptr));

    asapo::Error err;
    producer.GetLastStream(1000, &err);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kTimeout));
}


TEST_F(ProducerImplTests, ReturnDataIfCanotAddToQueue) {
    producer.SetCredentials(expected_credentials);

    asapo::MessageData data = asapo::MessageData{new uint8_t[100]};
    data[40] = 10;
    asapo::OriginalRequest* original_request = new asapo::OriginalRequest{};

    auto request = std::unique_ptr<ProducerRequest> {new ProducerRequest{"", asapo::GenericRequestHeader{},std::move(data), "", "", nullptr, true, 0}};
    original_request->request = std::move(request);
    auto pool_err = asapo::IOErrorTemplates::kNoSpaceLeft.Generate();
    pool_err->SetCustomData(std::unique_ptr<asapo::CustomErrorData>{original_request});


    EXPECT_CALL(mock_pull, AddRequest_t(_,_)).WillOnce(Return(
        std::move(pool_err).release()));

    asapo::MessageHeader message_header{expected_id, 0, expected_name};
    auto err = producer.Send(message_header, std::move(data), expected_ingest_mode, expected_stream, nullptr);

    auto err_data = static_cast<asapo::OriginalData*>(err->GetCustomData());
    ASSERT_THAT(err_data, Ne(nullptr));

    asapo::MessageData original_data_in_err = std::move(err_data->data);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kRequestPoolIsFull));
    ASSERT_THAT(original_data_in_err, Ne(nullptr));
    ASSERT_THAT(original_data_in_err[40], Eq(10));

}

TEST_F(ProducerImplTests, GetVersionInfoWithServer) {

    std::string result = R"({"softwareVersion":"20.03.1, build 7a9294ad","clientSupported":"no", "clientProtocol":{"versionInfo":"v0.2"}})";

    EXPECT_CALL(*mock_http_client, Get_t(HasSubstr(expected_server_uri + "/asapo-discovery/v0.1/version?client=producer&protocol=v0.1"), _,_)).WillOnce(DoAll(
        SetArgPointee<1>(asapo::HttpCode::OK),
        SetArgPointee<2>(nullptr),
        Return(result)));

    std::string client_info,server_info;
    auto err = producer.GetVersionInfo(&client_info,&server_info,nullptr);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(server_info, HasSubstr("20.03.1"));
    ASSERT_THAT(server_info, HasSubstr("v0.2"));
}

}
