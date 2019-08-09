#include <gtest/gtest.h>
#include <gmock/gmock.h>

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

class DbWriterHandlerTests : public Test {
  public:
    std::string expected_collection_name = asapo::kDBDataCollectionName;
    RequestHandlerDbWrite handler{expected_collection_name};
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<MockDatabase> mock_db;
    NiceMock<asapo::MockLogger> mock_logger;
    ReceiverConfig config;
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_default_stream = "detector";
    std::string expected_stream = "stream";
    std::string expected_hostname = "host";
    uint64_t expected_port = 1234;
    uint64_t expected_buf_id = 18446744073709551615ull;
    std::string expected_file_name = "2";
    std::string expected_metadata = "meta";
    uint64_t expected_file_size = 10;
    uint64_t expected_id = 15;
    uint64_t expected_subset_id = 15;
    uint64_t expected_subset_size = 2;
    uint64_t expected_custom_data[2] {expected_subset_id, expected_subset_size};
    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 2;
        request_header.op_code = asapo::Opcode::kOpcodeTransferData;
        handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
        handler.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, ""});
        config.broker_db_uri = "127.0.0.1:27017";
        config.source_host = expected_hostname;
        config.dataserver.listen_port = expected_port;
        SetReceiverConfig(config, "none");

        ON_CALL(*mock_request, GetBeamtimeId()).WillByDefault(ReturnRef(expected_beamtime_id));
    }
    void ExpectRequestParams(asapo::Opcode op_code, const std::string& stream);

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
    if (stream != "detector") {
        db_name += "_" + stream;
    }
    EXPECT_CALL(mock_db, Connect_t(config.broker_db_uri, db_name, expected_collection_name)).
    WillOnce(testing::Return(nullptr));

    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(expected_file_size))
    ;

    EXPECT_CALL(*mock_request, GetFileName())
    .WillOnce(Return(expected_file_name))
    ;

    EXPECT_CALL(*mock_request, GetMetaData())
    .WillOnce(ReturnRef(expected_metadata))
    ;

    EXPECT_CALL(*mock_request, GetDataID())
    .WillOnce(Return(expected_id))
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
    file_info.source = expected_hostname + ":" + std::to_string(expected_port);
    file_info.metadata = expected_metadata;
    return file_info;
}

TEST_F(DbWriterHandlerTests, CallsInsert) {

    ExpectRequestParams(asapo::Opcode::kOpcodeTransferData, expected_stream);
    auto file_info = PrepareFileInfo();

    EXPECT_CALL(mock_db, Insert_t(CompareFileInfo(file_info), _)).
    WillOnce(testing::Return(nullptr));

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("insert record"),
                                         HasSubstr(config.broker_db_uri),
                                         HasSubstr(expected_beamtime_id),
                                         HasSubstr(expected_stream),
                                         HasSubstr(expected_collection_name)
                                        )
                                  )
               );

    handler.ProcessRequest(mock_request.get());
}

TEST_F(DbWriterHandlerTests, CallsInsertSubset) {

    ExpectRequestParams(asapo::Opcode::kOpcodeTransferSubsetData, expected_default_stream);
    auto file_info = PrepareFileInfo();


    EXPECT_CALL(mock_db, InsertAsSubset_t(CompareFileInfo(file_info), expected_subset_id, expected_subset_size, _)).
    WillOnce(testing::Return(nullptr));

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("insert record"),
                                         HasSubstr(config.broker_db_uri),
                                         HasSubstr(expected_beamtime_id),
                                         HasSubstr(expected_collection_name)
                                        )
                                  )
               );

    handler.ProcessRequest(mock_request.get());
}


}