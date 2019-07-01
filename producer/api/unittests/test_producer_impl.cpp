#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockLogger.h"
#include "common/error.h"
#include "producer/common.h"
#include "../src/producer_impl.h"
#include "producer/producer_error.h"

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


MATCHER_P8(M_CheckSendDataRequest, op_code, beamtime_id, metadata, file_id, file_size, message, subset_id, subset_size,
           "Checks if a valid GenericRequestHeader was Send") {
    auto request = static_cast<ProducerRequest*>(arg);
    return ((asapo::GenericRequestHeader)(arg->header)).op_code == op_code
           && ((asapo::GenericRequestHeader)(arg->header)).data_id == file_id
           && ((asapo::GenericRequestHeader)(arg->header)).data_size == uint64_t(file_size)
           && request->beamtime_id == beamtime_id
           && request->metadata == metadata
           && (op_code == asapo::kOpcodeTransferMetaData ? ((asapo::GenericRequestHeader)(arg->header)).custom_data[0] ==
               uint64_t(subset_id) : true)
           && (op_code == asapo::kOpcodeTransferMetaData ? ((asapo::GenericRequestHeader)(arg->header)).custom_data[1] ==
               uint64_t(subset_size) : true)
           && strcmp(((asapo::GenericRequestHeader)(arg->header)).message, message) == 0;
}


TEST(ProducerImpl, Constructor) {
    asapo::ProducerImpl producer{"", 4, asapo::RequestHandlerType::kTcp};
    ASSERT_THAT(dynamic_cast<asapo::AbstractLogger*>(producer.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::RequestPool*>(producer.request_pool__.get()), Ne(nullptr));
}

class ProducerImplTests : public testing::Test {
  public:
    testing::NiceMock<MockDiscoveryService> service;
    asapo::ProducerRequestHandlerFactory factory{&service};
    testing::NiceMock<asapo::MockLogger> mock_logger;
    testing::NiceMock<MockRequestPull> mock_pull{&factory, &mock_logger};
    asapo::ProducerImpl producer{"", 1, asapo::RequestHandlerType::kTcp};
    uint64_t expected_size = 100;
    uint64_t expected_id = 10;
    uint64_t expected_subset_id = 100;
    uint64_t expected_subset_size = 4;

    char expected_name[asapo::kMaxMessageSize] = "test_name";
    std::string expected_beamtimeid = "beamtime_id";
    std::string expected_metadata = "meta";
    std::string expected_fullpath = "filename";
    void SetUp() override {
        producer.log__ = &mock_logger;
        producer.request_pool__ = std::unique_ptr<RequestPool> {&mock_pull};
    }
    void TearDown() override {
        producer.request_pool__.release();
    }
};

TEST_F(ProducerImplTests, SendReturnsError) {
    EXPECT_CALL(mock_pull, AddRequest_t(_)).WillOnce(Return(
            asapo::ProducerErrorTemplates::kRequestPoolIsFull.Generate().release()));
    asapo::EventHeader event_header{1, 1, ""};
    auto err = producer.SendData(event_header, nullptr, "", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kRequestPoolIsFull));
}

TEST_F(ProducerImplTests, ErrorIfFileNameTooLong) {
    std::string long_string(asapo::kMaxMessageSize + 100, 'a');
    asapo::EventHeader event_header{1, 1, long_string};
    auto err = producer.SendData(event_header, nullptr, "", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kFileNameTooLong));
}


TEST_F(ProducerImplTests, ErrorIfSizeTooLarge) {
    EXPECT_CALL(mock_logger, Error(testing::HasSubstr("error checking")));
    asapo::EventHeader event_header{1, asapo::ProducerImpl::kMaxChunkSize + 1, ""};
    auto err = producer.SendData(event_header, nullptr, "", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kFileTooLarge));
}

TEST_F(ProducerImplTests, ErrorIfSubsetSizeNotDefined) {
    EXPECT_CALL(mock_logger, Error(testing::HasSubstr("subset size")));
    asapo::EventHeader event_header{1, asapo::ProducerImpl::kMaxChunkSize, "", 1};
    auto err = producer.SendData(event_header, nullptr, "", nullptr);
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kErrorSubsetSize));
}



TEST_F(ProducerImplTests, OKSendingSendDataRequest) {
    producer.SetBeamtimeId(expected_beamtimeid);

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferData,
                                        expected_beamtimeid, expected_metadata,
                                        expected_id, expected_size, expected_name, 0, 0))).WillOnce(Return(
                                                    nullptr));

    asapo::EventHeader event_header{expected_id, expected_size, expected_name};
    auto err = producer.SendData(event_header, nullptr, expected_metadata, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKSendingSendSubsetDataRequest) {
    producer.SetBeamtimeId(expected_beamtimeid);
    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferSubsetData,
                                        expected_beamtimeid, expected_metadata,
                                        expected_id, expected_size, expected_name,
                                        expected_subset_id, expected_subset_size))).WillOnce(Return(
                                                    nullptr));

    asapo::EventHeader event_header{expected_id, expected_size, expected_name, expected_subset_id, expected_subset_size};
    auto err = producer.SendData(event_header, nullptr, expected_metadata, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}



TEST_F(ProducerImplTests, OKAddingSendMetaDataRequest) {
    expected_id = 0;
    expected_metadata = "{\"meta\":10}";
    expected_size = expected_metadata.size();

    producer.SetBeamtimeId(expected_beamtimeid);
    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferMetaData,
                                        expected_beamtimeid, "", expected_id,
                                        expected_size, "beamtime_global.meta", 0, 0))).WillOnce(Return(
                                                    nullptr));

    auto err = producer.SendMetaData(expected_metadata, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ProducerImplTests, OKSendingSendFileRequest) {
    producer.SetBeamtimeId(expected_beamtimeid);

    EXPECT_CALL(mock_pull, AddRequest_t(M_CheckSendDataRequest(asapo::kOpcodeTransferData,
                                        expected_beamtimeid, "", expected_id, 0, expected_name, 0, 0))).WillOnce(Return(
                                                    nullptr));

    asapo::EventHeader event_header{expected_id, 0, expected_name};
    auto err = producer.SendFile(event_header, expected_fullpath, nullptr);

    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(ProducerImplTests, ErrorSettingBeamtime) {
    std::string expected_beamtimeid(asapo::kMaxMessageSize * 10, 'a');
    EXPECT_CALL(mock_logger, Error(testing::HasSubstr("too long")));

    auto err = producer.SetBeamtimeId(expected_beamtimeid);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kBeamtimeIdTooLong));
}

TEST_F(ProducerImplTests, ErrorSettingSecondTime) {
    EXPECT_CALL(mock_logger, Error(testing::HasSubstr("already")));

    producer.SetBeamtimeId("1");
    auto err = producer.SetBeamtimeId("2");

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kBeamtimeAlreadySet));
}



}
