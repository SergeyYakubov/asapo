#pragma clang diagnostic push
#pragma ide diagnostic ignored "InfiniteRecursion"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockLogger.h"
#include "asapo/common/error.h"
#include "asapo/producer/common.h"
#include "../src/producer_impl.h"
#include "asapo/producer/producer_error.h"

#include "../src/request_handler_tcp.h"

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


using asapo::RequestPool;
using asapo::ProducerRequest;

MATCHER_P10(M_CheckSendDataRequest, op_code, source_credentials, metadata, file_id, file_size, message, stream,
            ingest_mode,
            subset_id,
            subset_size,
            "Checks if a valid GenericRequestHeader was Send") {
    auto request = static_cast<ProducerRequest*>(arg);
    return ((asapo::GenericRequestHeader) (arg->header)).op_code == op_code
        && ((asapo::GenericRequestHeader) (arg->header)).data_id == file_id
        && ((asapo::GenericRequestHeader) (arg->header)).data_size == uint64_t(file_size)
        && request->manage_data_memory == true
        && request->source_credentials == source_credentials
        && request->metadata == metadata
        && (op_code == asapo::kOpcodeTransferSubsetData ? ((asapo::GenericRequestHeader) (arg->header)).custom_data[1]
            == uint64_t(subset_id) : true)
        && (op_code == asapo::kOpcodeTransferSubsetData ? ((asapo::GenericRequestHeader) (arg->header)).custom_data[2]
            == uint64_t(subset_size) : true)
        && ((asapo::GenericRequestHeader) (arg->header)).custom_data[asapo::kPosIngestMode] == uint64_t(ingest_mode)
        && strcmp(((asapo::GenericRequestHeader) (arg->header)).message, message) == 0
        && strcmp(((asapo::GenericRequestHeader) (arg->header)).stream, stream) == 0;
}

TEST(ProducerImpl, Constructor) {
    asapo::ProducerImpl producer{"", 4, 3600000, asapo::RequestHandlerType::kTcp};
    ASSERT_THAT(dynamic_cast<asapo::AbstractLogger*>(producer.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::RequestPool*>(producer.request_pool__.get()), Ne(nullptr));
}

class ProducerImplTests : public testing::Test {
 public:
  testing::NiceMock<MockDiscoveryService> service;
  asapo::ProducerRequestHandlerFactory factory{&service};
  testing::NiceMock<asapo::MockLogger> mock_logger;
  testing::NiceMock<MockRequestPull> mock_pull{&factory, &mock_logger};
  asapo::ProducerImpl producer{"", 1, 3600000, asapo::RequestHandlerType::kTcp};
  uint64_t expected_size = 100;
  uint64_t expected_id = 10;
  uint64_t expected_subset_id = 100;
  uint64_t expected_subset_size = 4;
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
  void SetUp() override {
      producer.log__ = &mock_logger;
      producer.request_pool__ = std::unique_ptr<RequestPool>{&mock_pull};
  }
  void TearDown() override {
      producer.request_pool__.release();
  }
};

TEST_F(ProducerImplTests, SendReturnsError) {
    EXPECT_CALL(mock_pull, AddRequest_t(_, false)).WillOnce(Return(
        asapo::ProducerErrorTemplates::kRequestPoolIsFull.Generate().release()));
    asapo::EventHeader event_header{1, 1, "test"};
    auto err = producer.SendData(event_header, nullptr, expected_ingest_mode, nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kRequestPoolIsFull));
}

TEST_F(ProducerImplTests, ErrorIfFileNameTooLong) {
    std::string long_string(asapo::kMaxMessageSize + 100, 'a');
    asapo::EventHeader event_header{1, 1, long_string};
    auto err = producer.SendData(event_header, nullptr, expected_ingest_mode, nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, ErrorIfFileEmpty) {
    std::string long_string(asapo::kMaxMessageSize + 100, 'a');
    asapo::EventHeader event_header{1, 1, ""};
    auto err = producer.SendData(event_header, nullptr, expected_ingest_mode, nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, ErrorIfSubsetSizeNotDefined) {
    EXPECT_CALL(mock_logger, Error(testing::HasSubstr("subset dimensions")));
    asapo::EventHeader event_header{1, 1000, "test", "", 1};
    auto err = producer.SendData(event_header, nullptr, expected_ingest_mode, nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, ErrorIfZeroDataSize) {
    asapo::FileData data = asapo::FileData{new uint8_t[100]};
    asapo::EventHeader event_header{1, 0, expected_fullpath};
    auto err = producer.SendData(event_header, std::move(data), asapo::kDefaultIngestMode, nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, ErrorIfNoData) {
    asapo::EventHeader event_header{1, 100, expected_fullpath};
    auto err = producer.SendData(event_header, nullptr, asapo::kDefaultIngestMode, nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, ErrorIfNoDataSend_) {
    asapo::EventHeader event_header{1, 100, expected_fullpath};
    auto err = producer.SendData__(event_header, nullptr, asapo::kDefaultIngestMode, nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, ErrorIfSendingDataWithZeroId) {
    asapo::EventHeader event_header{0, 100, expected_fullpath};
    auto err = producer.SendData(event_header, nullptr, asapo::kTransferMetaDataOnly, nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST_F(ProducerImplTests, OkIfNoDataWithTransferMetadataOnlyMode) {
    asapo::EventHeader event_header{1, 100, expected_fullpath};
    auto err = producer.SendData(event_header, nullptr, asapo::kTransferMetaDataOnly, nullptr);
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OkIfZeroSizeWithTransferMetadataOnlyMode) {
    asapo::FileData data = asapo::FileData{new uint8_t[100]};
    asapo::EventHeader event_header{1, 0, expected_fullpath};
    auto err = producer.SendData(event_header, std::move(data), asapo::kTransferMetaDataOnly, nullptr);
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, UsesDefaultStream) {
    producer.SetCredentials(expected_default_credentials);

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferData,
                                                               expected_default_credentials_str,
                                                               expected_metadata,
                                                               expected_id,
                                                               expected_size,
                                                               expected_name,
                                                               asapo::kDefaultStream.c_str(),
                                                               expected_ingest_mode,
                                                               0,
                                                               0), false)).WillOnce(Return(nullptr));

    asapo::EventHeader event_header{expected_id, expected_size, expected_name, expected_metadata};
    auto err = producer.SendData(event_header, nullptr, expected_ingest_mode, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKSendingSendDataRequest) {
    producer.SetCredentials(expected_credentials);

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferData,
                                                               expected_credentials_str,
                                                               expected_metadata,
                                                               expected_id,
                                                               expected_size,
                                                               expected_name,
                                                               asapo::kDefaultStream.c_str(),
                                                               expected_ingest_mode,
                                                               0,
                                                               0
    ), false)).WillOnce(Return(
        nullptr));

    asapo::EventHeader event_header{expected_id, expected_size, expected_name, expected_metadata};
    auto err = producer.SendData(event_header, nullptr, expected_ingest_mode, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKSendingSendDataRequestWithStream) {
    producer.SetCredentials(expected_credentials);

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferData,
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

    asapo::EventHeader event_header{expected_id, expected_size, expected_name, expected_metadata};
    auto err = producer.SendData(event_header, expected_stream, nullptr, expected_ingest_mode, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKSendingStreamFinish) {
    producer.SetCredentials(expected_credentials);

    std::string next_stream_meta = std::string("{\"next_stream\":") + "\"" + expected_next_stream + "\"}";

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferData,
                                                               expected_credentials_str,
                                                               next_stream_meta.c_str(),
                                                               expected_id + 1,
                                                               0,
                                                               asapo::ProducerImpl::kFinishStreamKeyword.c_str(),
                                                               expected_stream,
                                                               asapo::IngestModeFlags::kTransferMetaDataOnly,
                                                               0,
                                                               0
    ), false)).WillOnce(Return(
        nullptr));

    auto err = producer.SendStreamFinishedFlag(expected_stream, expected_id, expected_next_stream, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKSendingStreamFinishWithNoNextStream) {
    producer.SetCredentials(expected_credentials);

    std::string
        next_stream_meta = std::string("{\"next_stream\":") + "\"" + asapo::ProducerImpl::kNoNextStreamKeyword
        + "\"}";

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferData,
                                                               expected_credentials_str,
                                                               next_stream_meta.c_str(),
                                                               expected_id + 1,
                                                               0,
                                                               asapo::ProducerImpl::kFinishStreamKeyword.c_str(),
                                                               expected_stream,
                                                               asapo::IngestModeFlags::kTransferMetaDataOnly,
                                                               0,
                                                               0
    ), false)).WillOnce(Return(
        nullptr));

    auto err = producer.SendStreamFinishedFlag(expected_stream, expected_id, "", nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKSendingSendSubsetDataRequest) {
    producer.SetCredentials(expected_credentials);
    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferSubsetData,
                                                               expected_credentials_str,
                                                               expected_metadata,
                                                               expected_id,
                                                               expected_size,
                                                               expected_name,
                                                               asapo::kDefaultStream.c_str(),
                                                               expected_ingest_mode,
                                                               expected_subset_id,
                                                               expected_subset_size), false)).WillOnce(
        Return(
            nullptr));

    asapo::EventHeader event_header
        {expected_id, expected_size, expected_name, expected_metadata, expected_subset_id, expected_subset_size};
    auto err = producer.SendData(event_header, nullptr, expected_ingest_mode, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKAddingSendMetaDataRequest) {
    expected_id = 0;
    expected_metadata = "{\"meta\":10}";
    expected_size = expected_metadata.size();
    expected_ingest_mode = asapo::IngestModeFlags::kTransferData | asapo::IngestModeFlags::kStoreInDatabase ;

    producer.SetCredentials(expected_credentials);
    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferMetaData,
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

    auto err = producer.SendMetaData(expected_metadata, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, ErrorSendingEmptyFileName) {
    producer.SetCredentials(expected_credentials);

    EXPECT_CALL(mock_pull, AddRequest_t(_, _)).Times(0);

    asapo::EventHeader event_header{expected_id, 0, expected_name};
    auto err = producer.SendFile(event_header, "", expected_ingest_mode, nullptr);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));

}

TEST_F(ProducerImplTests, ErrorSendingEmptyRelativeFileName) {
    producer.SetCredentials(expected_credentials);

    EXPECT_CALL(mock_pull, AddRequest_t(_, _)).Times(0);

    asapo::EventHeader event_header{expected_id, 0, ""};
    auto err = producer.SendFile(event_header, expected_fullpath, expected_ingest_mode, nullptr);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));

}

TEST_F(ProducerImplTests, OKSendingSendFileRequest) {
    producer.SetCredentials(expected_credentials);

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferData,
                                                               expected_credentials_str,
                                                               "",
                                                               expected_id,
                                                               0,
                                                               expected_name,
                                                               asapo::kDefaultStream.c_str(),
                                                               expected_ingest_mode,
                                                               0,
                                                               0), false)).WillOnce(Return(
        nullptr));

    asapo::EventHeader event_header{expected_id, 0, expected_name};
    auto err = producer.SendFile(event_header, expected_fullpath, expected_ingest_mode, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKSendingSendFileRequestWithStream) {
    producer.SetCredentials(expected_credentials);

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferData,
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

    asapo::EventHeader event_header{expected_id, 0, expected_name};
    auto err = producer.SendFile(event_header, expected_stream, expected_fullpath, expected_ingest_mode, nullptr);

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
    asapo::EventHeader event_header{expected_id, 0, expected_name};
    uint64_t ingest_modes[] = {0, asapo::IngestModeFlags::kTransferMetaDataOnly | asapo::IngestModeFlags::kTransferData,
                               asapo::IngestModeFlags::kTransferData,
                               asapo::IngestModeFlags::kStoreInDatabase,
                               asapo::IngestModeFlags::kStoreInFilesystem,
                               asapo::IngestModeFlags::kStoreInDatabase | asapo::IngestModeFlags::kStoreInFilesystem,
                               asapo::IngestModeFlags::kTransferMetaDataOnly
                                   | asapo::IngestModeFlags::kStoreInFilesystem};

    EXPECT_CALL(mock_pull, AddRequest_t(_, _)).Times(0);

    for (auto ingest_mode : ingest_modes) {
        auto err = producer.SendFile(event_header, expected_fullpath, ingest_mode, nullptr);
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

TEST(GetStreamInfoTest, GetStreamInfoTimeout) {
    asapo::ProducerImpl producer1{"", 1, 10000, asapo::RequestHandlerType::kTcp};
    asapo::Error err;
    auto sinfo = producer1.GetStreamInfo(5000, &err);

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

}

#pragma clang diagnostic pop