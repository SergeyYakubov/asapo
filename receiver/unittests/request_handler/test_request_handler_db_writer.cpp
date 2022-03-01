#include <chrono>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "asapo/database/db_error.h"


#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockDatabase.h"
#include "asapo/unittests/MockLogger.h"

#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/request_handler/request_factory.h"
#include "../../src/request_handler/request_handler.h"
#include "../../src/request_handler/request_handler_db_write.h"
#include "asapo/common/networking.h"
#include "../../../common/cpp/src/database/mongodb_client.h"
#include "../mock_receiver_config.h"
#include "asapo/common/data_structs.h"

#include "../receiver_mocking.h"

using namespace testing;
using namespace asapo;


namespace {

TEST(DbWriterHandler, Constructor) {
    RequestHandlerDbWrite handler{""};
    ASSERT_THAT(dynamic_cast<asapo::HttpClient*>(handler.http_client__.get()), Ne(nullptr));
}



class DbWriterHandlerTests : public Test {
  public:
    std::string expected_stream = "stream";
    std::string expected_collection_name = std::string(asapo::kDBDataCollectionNamePrefix) + "_" + expected_stream;
    RequestHandlerDbWrite handler{asapo::kDBDataCollectionNamePrefix};
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<MockDatabase> mock_db;
    NiceMock<asapo::MockLogger> mock_logger;
    ReceiverConfig config;
    std::string expected_beamtime_id = "beamtime_id";
    uint64_t expected_ingest_mode = asapo::kDefaultIngestMode;
    std::string expected_default_source = "detector";
    std::string expected_data_source = "source";
    std::string expected_host_ip = "127.0.0.1";
    uint64_t expected_port = 1234;
    uint64_t expected_buf_id = 18446744073709551615ull;
    std::string expected_file_name = "2";
    std::string expected_metadata = "meta";
    uint64_t expected_file_size = 10;
    uint64_t expected_id = 15;
    uint64_t expected_substream = 20;
    uint64_t expected_dataset_size = 2;
    uint64_t expected_custom_data[asapo::kNCustomParams] {0, expected_substream, expected_dataset_size};
    asapo::MockHandlerDbCheckRequest mock_db_check_handler{asapo::kDBDataCollectionNamePrefix};

    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 2;
        request_header.op_code = asapo::Opcode::kOpcodeTransferData;
        handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
        handler.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", &mock_db_check_handler, nullptr});
        config.database_uri = "127.0.0.1:27017";
        config.dataserver.advertise_uri = expected_host_ip + ":" + std::to_string(expected_port);
        config.dataserver.listen_port = expected_port;
        SetReceiverConfig(config, "none");

        SetDefaultRequestCalls(mock_request.get(),expected_beamtime_id);
    }
    void ExpectRequestParams(asapo::Opcode op_code, const std::string& data_source);
    void ExpectLogger();
    void ExpectDuplicatedID();
    MessageMeta PrepareMessageMeta(bool substream = false);
    void TearDown() override {
        handler.db_client__.release();
    }


};

MATCHER_P(CompareMessageMeta, file, "") {
    if (arg.size != file.size) return false;
    if (arg.source != file.source) return false;
    if (arg.buf_id != file.buf_id) return false;
    if (arg.dataset_substream != file.dataset_substream) return false;
    if (arg.name != file.name) return false;
    if (arg.stream != file.stream) return false;
    if (arg.ingest_mode != file.ingest_mode) return false;
    if (arg.id != file.id) return false;
    if (arg.metadata != file.metadata) return false;

    if (arg.timestamp < std::chrono::system_clock::now() - std::chrono::seconds (5)) {
        return false;
    }

    return true;
}


void DbWriterHandlerTests::ExpectRequestParams(asapo::Opcode op_code, const std::string& data_source) {

    EXPECT_CALL(*mock_request, WasAlreadyProcessed())
    .WillOnce(Return(false))
    ;

    EXPECT_CALL(*mock_request, GetBeamtimeId())
    .WillRepeatedly(ReturnRef(expected_beamtime_id))
    ;

    EXPECT_CALL(*mock_request, GetDataSource())
    .WillRepeatedly(ReturnRef(data_source))
    ;

    EXPECT_CALL(*mock_request, GetIngestMode())
        .WillRepeatedly(Return(expected_ingest_mode))
        ;

    EXPECT_CALL(*mock_request, GetSlotId())
    .WillOnce(Return(expected_buf_id))
    ;

    std::string db_name = expected_beamtime_id;
    db_name += "_" + data_source;

    EXPECT_CALL(mock_db, Connect_t(config.database_uri, db_name)).
    WillOnce(testing::Return(nullptr));

    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(expected_file_size))
    ;

    EXPECT_CALL(*mock_request, GetFileName())
    .WillOnce(Return(expected_file_name))
    ;

    EXPECT_CALL(*mock_request, GetStream())
    .WillRepeatedly(Return(expected_stream))
    ;

    EXPECT_CALL(*mock_request, GetMetaData())
    .WillOnce(ReturnRef(expected_metadata))
    ;

    EXPECT_CALL(*mock_request, GetDataID()).Times(AtLeast(1)).WillRepeatedly
    (Return(expected_id))
    ;

    EXPECT_CALL(*mock_request, GetOpCode())
    .WillRepeatedly(Return(op_code))
    ;

    if (op_code == asapo::Opcode::kOpcodeTransferDatasetData) {
        EXPECT_CALL(*mock_request, GetCustomData_t()).
        WillRepeatedly(Return(expected_custom_data))
        ;
    }
}




MessageMeta DbWriterHandlerTests::PrepareMessageMeta(bool substream) {
    MessageMeta message_meta;
    message_meta.size = expected_file_size;
    message_meta.name = expected_file_name;
    message_meta.id = expected_id;
    message_meta.ingest_mode = expected_ingest_mode;
    if (substream) {
        message_meta.dataset_substream = expected_substream;
    }
    message_meta.buf_id = expected_buf_id;
    message_meta.source = expected_host_ip + ":" + std::to_string(expected_port);
    message_meta.stream = expected_stream;
    message_meta.metadata = expected_metadata;
    return message_meta;
}
void DbWriterHandlerTests::ExpectLogger() {
    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("insert"),
                                         HasSubstr(expected_beamtime_id)
                                        )
                                  )
               );

}

TEST_F(DbWriterHandlerTests, CallsInsert) {

    ExpectRequestParams(asapo::Opcode::kOpcodeTransferData, expected_data_source);
    auto message_meta = PrepareMessageMeta();

    EXPECT_CALL(mock_db, Insert_t(expected_collection_name, CompareMessageMeta(message_meta), false, _)).
    WillOnce(testing::Return(nullptr));
    ExpectLogger();

    handler.ProcessRequest(mock_request.get());
}

TEST_F(DbWriterHandlerTests, CallsInsertDataset) {

    ExpectRequestParams(asapo::Opcode::kOpcodeTransferDatasetData, expected_data_source);
    auto message_meta = PrepareMessageMeta(true);


    EXPECT_CALL(mock_db, InsertAsDatasetMessage_t(expected_collection_name, CompareMessageMeta(message_meta),
                                                  expected_dataset_size, false)).
    WillOnce(testing::Return(   nullptr));
    ExpectLogger();

    handler.ProcessRequest(mock_request.get());
}


void DbWriterHandlerTests::ExpectDuplicatedID() {
    ExpectRequestParams(asapo::Opcode::kOpcodeTransferData, expected_data_source);
    auto message_meta = PrepareMessageMeta();

    EXPECT_CALL(mock_db, Insert_t(expected_collection_name, CompareMessageMeta(message_meta), false, _)).
    WillOnce(testing::Return(asapo::DBErrorTemplates::kDuplicateID.Generate().release()));
}

TEST_F(DbWriterHandlerTests, SkipIfWasAlreadyProcessed) {
    EXPECT_CALL(*mock_request, WasAlreadyProcessed())
    .WillOnce(Return(true));

    EXPECT_CALL(*mock_request, GetBeamtimeId()).Times(0);

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));

}

TEST_F(DbWriterHandlerTests, DuplicatedRequest_SameRecord) {
    ExpectDuplicatedID();

    EXPECT_CALL(*mock_request, SetResponseMessage(HasSubstr("duplicate record"), asapo::ResponseMessageType::kWarning));
    EXPECT_CALL(*mock_request, CheckForDuplicates_t())
    .WillOnce(
        Return(asapo::ReceiverErrorTemplates::kWarningDuplicatedRequest.Generate().release())
    );

    EXPECT_CALL(mock_logger, Warning(HasSubstr("ignoring")));

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));

}

TEST_F(DbWriterHandlerTests, DuplicatedRequest_DifferentRecord) {
    ExpectDuplicatedID();

    EXPECT_CALL(*mock_request, CheckForDuplicates_t())
    .WillOnce(
        Return(asapo::ReceiverErrorTemplates::kBadRequest.Generate().release())
    );

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kBadRequest));

}


}
