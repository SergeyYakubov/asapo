#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <database/db_error.h>

#include "unittests/MockIO.h"
#include "unittests/MockDatabase.h"
#include "unittests/MockLogger.h"

#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_factory.h"
#include "../src/request_handler.h"
#include "../src/request_handler_db_write.h"
#include "common/networking.h"
#include "../../common/cpp/src/database/mongodb_client.h"
#include "mock_receiver_config.h"
#include "common/data_structs.h"

#include "receiver_mocking.h"

using asapo::MockRequest;
using asapo::FileInfo;
using ::testing::Test;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::InSequence;
using ::testing::SetArgPointee;
using ::testing::AllOf;
using ::testing::HasSubstr;
using ::testing::AtLeast;


using ::asapo::Error;
using ::asapo::ErrorInterface;
using ::asapo::FileDescriptor;
using ::asapo::SocketDescriptor;
using ::asapo::MockIO;
using asapo::Request;
using asapo::RequestHandlerDbWrite;
using ::asapo::GenericRequestHeader;

using asapo::MockDatabase;
using asapo::RequestFactory;
using asapo::SetReceiverConfig;
using asapo::ReceiverConfig;


namespace {

TEST(DbWriterHandler, Constructor) {
    RequestHandlerDbWrite handler{""};
    ASSERT_THAT(dynamic_cast<asapo::HttpClient*>(handler.http_client__.get()), Ne(nullptr));
}



class DbWriterHandlerTests : public Test {
  public:
    std::string expected_substream = "substream";
    std::string expected_collection_name = std::string(asapo::kDBDataCollectionNamePrefix) + "_" + expected_substream;
    RequestHandlerDbWrite handler{asapo::kDBDataCollectionNamePrefix};
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<MockDatabase> mock_db;
    NiceMock<asapo::MockLogger> mock_logger;
    ReceiverConfig config;
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_default_stream = "detector";
    std::string expected_stream = "stream";
    std::string expected_host_ip = "127.0.0.1";
    uint64_t expected_port = 1234;
    uint64_t expected_buf_id = 18446744073709551615ull;
    std::string expected_file_name = "2";
    std::string expected_metadata = "meta";
    uint64_t expected_file_size = 10;
    uint64_t expected_id = 15;
    uint64_t expected_subset_id = 15;
    uint64_t expected_subset_size = 2;
    uint64_t expected_custom_data[asapo::kNCustomParams] {0, expected_subset_id, expected_subset_size};
    asapo::MockHandlerDbCheckRequest mock_db_check_handler{asapo::kDBDataCollectionNamePrefix};

    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 2;
        request_header.op_code = asapo::Opcode::kOpcodeTransferData;
        handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
        handler.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", &mock_db_check_handler});
        config.database_uri = "127.0.0.1:27017";
        config.advertise_ip = expected_host_ip;
        config.dataserver.listen_port = expected_port;
        SetReceiverConfig(config, "none");

        ON_CALL(*mock_request, GetBeamtimeId()).WillByDefault(ReturnRef(expected_beamtime_id));
    }
    void ExpectRequestParams(asapo::Opcode op_code, const std::string& stream);
    void ExpectLogger();
    void ExpectDuplicatedID();
    FileInfo PrepareFileInfo();
    void TearDown() override {
        handler.db_client__.release();
    }


};

MATCHER_P(CompareFileInfo, file, "") {
    if (arg.size != file.size) return false;
    if (arg.source != file.source) return false;
    if (arg.buf_id != file.buf_id) return false;
    if (arg.name != file.name) return false;
    if (arg.id != file.id) return false;
    if (arg.metadata != file.metadata) return false;

    return true;
}


void DbWriterHandlerTests::ExpectRequestParams(asapo::Opcode op_code, const std::string& stream) {

    EXPECT_CALL(*mock_request, WasAlreadyProcessed())
    .WillOnce(Return(false))
    ;

    EXPECT_CALL(*mock_request, GetBeamtimeId())
    .WillOnce(ReturnRef(expected_beamtime_id))
    ;

    EXPECT_CALL(*mock_request, GetStream())
    .WillOnce(ReturnRef(stream))
    ;


    EXPECT_CALL(*mock_request, GetSlotId())
    .WillOnce(Return(expected_buf_id))
    ;

    std::string db_name = expected_beamtime_id;
    db_name += "_" + stream;

    EXPECT_CALL(mock_db, Connect_t(config.database_uri, db_name)).
    WillOnce(testing::Return(nullptr));

    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(expected_file_size))
    ;

    EXPECT_CALL(*mock_request, GetFileName())
    .WillOnce(Return(expected_file_name))
    ;

    EXPECT_CALL(*mock_request, GetSubstream())
    .WillOnce(Return(expected_substream))
    ;


    EXPECT_CALL(*mock_request, GetMetaData())
    .WillOnce(ReturnRef(expected_metadata))
    ;

    EXPECT_CALL(*mock_request, GetDataID()).Times(AtLeast(1)).WillRepeatedly
    (Return(expected_id))
    ;

    EXPECT_CALL(*mock_request, GetOpCode())
    .WillOnce(Return(op_code))
    ;

    if (op_code == asapo::Opcode::kOpcodeTransferSubsetData) {
        EXPECT_CALL(*mock_request, GetCustomData_t()).Times(2).
        WillRepeatedly(Return(expected_custom_data))
        ;
    }
}




FileInfo DbWriterHandlerTests::PrepareFileInfo() {
    FileInfo file_info;
    file_info.size = expected_file_size;
    file_info.name = expected_file_name;
    file_info.id = expected_id;
    file_info.buf_id = expected_buf_id;
    file_info.source = expected_host_ip + ":" + std::to_string(expected_port);
    file_info.metadata = expected_metadata;
    return file_info;
}
void DbWriterHandlerTests::ExpectLogger() {
    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("insert record"),
                                         HasSubstr(config.database_uri),
                                         HasSubstr(expected_beamtime_id),
                                         HasSubstr(expected_stream),
                                         HasSubstr(expected_collection_name)
                                        )
                                  )
               );

}

TEST_F(DbWriterHandlerTests, CallsInsert) {

    ExpectRequestParams(asapo::Opcode::kOpcodeTransferData, expected_stream);
    auto file_info = PrepareFileInfo();

    EXPECT_CALL(mock_db, Insert_t(expected_collection_name, CompareFileInfo(file_info), false)).
    WillOnce(testing::Return(nullptr));
    ExpectLogger();

    handler.ProcessRequest(mock_request.get());
}

TEST_F(DbWriterHandlerTests, CallsInsertSubset) {

    ExpectRequestParams(asapo::Opcode::kOpcodeTransferSubsetData, expected_default_stream);
    auto file_info = PrepareFileInfo();


    EXPECT_CALL(mock_db, InsertAsSubset_t(expected_collection_name, CompareFileInfo(file_info), expected_subset_id,
                                          expected_subset_size, false)).
    WillOnce(testing::Return(nullptr));
    ExpectLogger();

    handler.ProcessRequest(mock_request.get());
}


void DbWriterHandlerTests::ExpectDuplicatedID() {
    ExpectRequestParams(asapo::Opcode::kOpcodeTransferData, expected_stream);
    auto file_info = PrepareFileInfo();

    EXPECT_CALL(mock_db, Insert_t(expected_collection_name, CompareFileInfo(file_info), false)).
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

    EXPECT_CALL(*mock_request, SetWarningMessage(HasSubstr("duplicate record")));
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