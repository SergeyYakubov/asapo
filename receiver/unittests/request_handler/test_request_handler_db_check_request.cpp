#include <functional>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <database/db_error.h>

#include "unittests/MockIO.h"
#include "unittests/MockDatabase.h"
#include "unittests/MockLogger.h"

#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/request_handler/request_factory.h"
#include "../../src/request_handler/request_handler.h"
#include "../../src/request_handler/request_handler_db_check_request.h"
#include "common/networking.h"
#include "../../../common/cpp/src/database/mongodb_client.h"
#include "../mock_receiver_config.h"
#include "common/data_structs.h"

#include "../receiver_mocking.h"

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
using asapo::RequestHandlerDbCheckRequest;
using ::asapo::GenericRequestHeader;

using asapo::MockDatabase;
using asapo::RequestFactory;
using asapo::SetReceiverConfig;
using asapo::ReceiverConfig;

using MockFunctions = std::vector<std::function<void(asapo::ErrorInterface*, bool )>>;

namespace {

TEST(DbCheckRequestHandler, Constructor) {
    RequestHandlerDbCheckRequest handler{""};
    ASSERT_THAT(dynamic_cast<asapo::HttpClient*>(handler.http_client__.get()), Ne(nullptr));
}


class DbCheckRequestHandlerTests : public Test {
  public:
    std::string expected_substream = "substream";
    std::string expected_collection_name = std::string(asapo::kDBDataCollectionNamePrefix) + "_" + expected_substream;
    RequestHandlerDbCheckRequest handler{asapo::kDBDataCollectionNamePrefix};
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
    uint64_t expected_subset_id = 16;
    uint64_t expected_subset_size = 2;
    uint64_t expected_custom_data[asapo::kNCustomParams] {0, expected_subset_id, expected_subset_size};
    FileInfo expected_file_info;
    MockFunctions mock_functions;
    int n_run = 0;
    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 2;
        request_header.op_code = asapo::Opcode::kOpcodeTransferData;
        handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
        handler.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", nullptr});
        config.database_uri = "127.0.0.1:27017";
        config.advertise_ip = expected_host_ip;
        config.dataserver.listen_port = expected_port;
        SetReceiverConfig(config, "none");
        expected_file_info =  PrepareFileInfo();
        mock_functions.push_back([this](asapo::ErrorInterface * error, bool expect_compare) {
            MockGetByID(error, expect_compare);
            n_run++;
        });
        mock_functions.push_back([this](asapo::ErrorInterface * error, bool expect_compare) {
            MockGetSetByID(error, expect_compare);
            n_run++;
        });

        ON_CALL(*mock_request, GetBeamtimeId()).WillByDefault(ReturnRef(expected_beamtime_id));
    }
    void ExpectRequestParams(asapo::Opcode op_code, const std::string& stream, bool expect_compare = true);

    FileInfo PrepareFileInfo();
    void MockGetByID(asapo::ErrorInterface* error, bool expect_compare);
    void MockGetSetByID(asapo::ErrorInterface* error, bool expect_compare);
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


void DbCheckRequestHandlerTests::ExpectRequestParams(asapo::Opcode op_code, const std::string& stream,
        bool expect_compare) {

    std::string db_name = expected_beamtime_id;
    db_name += "_" + stream;

    if (n_run  == 0) {
        EXPECT_CALL(mock_db, Connect_t(config.database_uri, db_name)).
        WillOnce(testing::Return(nullptr));
        EXPECT_CALL(*mock_request, GetBeamtimeId())
        .WillOnce(ReturnRef(expected_beamtime_id))
        ;

        EXPECT_CALL(*mock_request, GetStream())
        .WillOnce(ReturnRef(stream))
        ;
    }


    if (expect_compare) {
        EXPECT_CALL(*mock_request, GetDataSize())
        .WillOnce(Return(expected_file_size))
        ;

        EXPECT_CALL(*mock_request, GetFileName())
        .WillOnce(Return(expected_file_name))
        ;

        EXPECT_CALL(*mock_request, GetMetaData())
        .WillOnce(ReturnRef(expected_metadata))
        ;
    }


    EXPECT_CALL(*mock_request, GetSubstream())
    .WillOnce(Return(expected_substream))
    ;



    EXPECT_CALL(*mock_request, GetDataID())
    .WillOnce(Return(expected_id))
    ;

    EXPECT_CALL(*mock_request, GetOpCode())
    .WillOnce(Return(op_code))
    ;

    if (op_code == asapo::Opcode::kOpcodeTransferSubsetData) {
        EXPECT_CALL(*mock_request, GetCustomData_t())
        .WillOnce(Return(expected_custom_data))
        ;
    }
}

FileInfo DbCheckRequestHandlerTests::PrepareFileInfo() {
    FileInfo file_info;
    file_info.size = expected_file_size;
    file_info.name = expected_file_name;
    file_info.id = expected_id;
    file_info.buf_id = expected_buf_id;
    file_info.source = expected_host_ip + ":" + std::to_string(expected_port);
    file_info.metadata = expected_metadata;
    return file_info;
}

void DbCheckRequestHandlerTests::MockGetByID(asapo::ErrorInterface* error, bool expect_compare ) {
    ExpectRequestParams(asapo::Opcode::kOpcodeTransferData, expected_stream, expect_compare);
    EXPECT_CALL(mock_db, GetById_t(expected_collection_name, expected_id, _)).
    WillOnce(DoAll(
                 SetArgPointee<2>(expected_file_info),
                 testing::Return(error)
             ));
}

void DbCheckRequestHandlerTests::MockGetSetByID(asapo::ErrorInterface* error, bool expect_compare ) {
    ExpectRequestParams(asapo::Opcode::kOpcodeTransferSubsetData, expected_stream, expect_compare);
    EXPECT_CALL(mock_db, GetSetById_t(expected_collection_name, expected_subset_id, expected_id, _)).
    WillOnce(DoAll(
                 SetArgPointee<3>(expected_file_info),
                 testing::Return(error)
             ));
}


TEST_F(DbCheckRequestHandlerTests, ErrorIfRecordsDoNotMatch) {
    expected_file_info.metadata = expected_metadata + "_";

    for (auto mock : mock_functions) {
        mock(nullptr, true);
        auto err = handler.ProcessRequest(mock_request.get());
        ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kBadRequest));
        Mock::VerifyAndClearExpectations(mock_request.get());
    }

}

TEST_F(DbCheckRequestHandlerTests, DuplicateErrorIfRecordsMatch) {
    for (auto mock : mock_functions) {
        mock(nullptr, true);
        auto err = handler.ProcessRequest(mock_request.get());
        ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kWarningDuplicatedRequest));
        Mock::VerifyAndClearExpectations(mock_request.get());
    }
}

TEST_F(DbCheckRequestHandlerTests, DuplicateErrorIfRecordsMatchWithEmptyMetadata) {
    expected_file_info.metadata = "{}";
    expected_metadata = "";
    for (auto mock : mock_functions) {
        mock(nullptr, true);
        auto err = handler.ProcessRequest(mock_request.get());
        ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kWarningDuplicatedRequest));
        Mock::VerifyAndClearExpectations(mock_request.get());
    }
}

TEST_F(DbCheckRequestHandlerTests, OkIfNotFound) {
    for (auto mock : mock_functions) {
        mock(asapo::DBErrorTemplates::kNoRecord.Generate().release(), false);
        auto err = handler.ProcessRequest(mock_request.get());
        ASSERT_THAT(err, Eq(nullptr));
        Mock::VerifyAndClearExpectations(mock_request.get());
    }
}

TEST_F(DbCheckRequestHandlerTests, ErrorIfDbError) {
    for (auto mock : mock_functions) {
        mock(asapo::DBErrorTemplates::kConnectionError.Generate().release(), false);
        auto err = handler.ProcessRequest(mock_request.get());
        ASSERT_THAT(err, Eq(asapo::DBErrorTemplates::kConnectionError));
        Mock::VerifyAndClearExpectations(mock_request.get());
    }
}

}
