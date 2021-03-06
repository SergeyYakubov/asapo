#include <functional>

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
#include "../../src/request_handler/request_handler_db_check_request.h"
#include "asapo/common/networking.h"
#include "../../../common/cpp/src/database/mongodb_client.h"
#include "../mock_receiver_config.h"
#include "asapo/common/data_structs.h"

#include "../receiver_mocking.h"

using namespace testing;
using namespace asapo;

using MockFunctions = std::vector<std::function<void(asapo::ErrorInterface*, bool )>>;

namespace {

TEST(DbCheckRequestHandler, Constructor) {
    RequestHandlerDbCheckRequest handler{""};
    ASSERT_THAT(dynamic_cast<asapo::HttpClient*>(handler.http_client__.get()), Ne(nullptr));
}


class DbCheckRequestHandlerTests : public Test {
  public:
    std::string expected_stream = "stream";
    std::string expected_collection_name = std::string(asapo::kDBDataCollectionNamePrefix) + "_" + expected_stream;
    RequestHandlerDbCheckRequest handler{asapo::kDBDataCollectionNamePrefix};
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<MockDatabase> mock_db;
    NiceMock<asapo::MockLogger> mock_logger;
    ReceiverConfig config;
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_default_source = "detector";
    std::string expected_data_source = "source";
    std::string expected_host_uri = "127.0.0.1:1234";
    uint64_t expected_port = 1234;
    uint64_t expected_buf_id = 18446744073709551615ull;
    std::string expected_file_name = "2";
    std::string expected_metadata = "meta";
    uint64_t expected_file_size = 10;
    uint64_t expected_id = 15;
    uint64_t expected_dataset_id = 16;
    uint64_t expected_dataset_size = 2;
    CustomRequestData expected_custom_data{0, expected_dataset_id, expected_dataset_size};
    MessageMeta expected_message_meta;
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
        config.dataserver.advertise_uri = expected_host_uri;
        config.dataserver.listen_port = expected_port;
        SetReceiverConfig(config, "none");
        expected_message_meta =  PrepareMessageMeta();
        mock_functions.push_back([this](asapo::ErrorInterface * error, bool expect_compare) {
            MockGetByID(error, expect_compare);
            n_run++;
        });
        mock_functions.push_back([this](asapo::ErrorInterface * error, bool expect_compare) {
            MockGetSetByID(error, expect_compare);
            n_run++;
        });
        SetDefaultRequestCalls(mock_request.get(),expected_beamtime_id);
    }
    void ExpectRequestParams(asapo::Opcode op_code, const std::string& data_source, bool expect_compare = true);

    MessageMeta PrepareMessageMeta();
    void MockGetByID(asapo::ErrorInterface* error, bool expect_compare);
    void MockGetSetByID(asapo::ErrorInterface* error, bool expect_compare);
    void TearDown() override {
        handler.db_client__.release();
    }


};

MATCHER_P(CompareMessageMeta, file, "") {
    if (arg.size != file.size) return false;
    if (arg.source != file.source) return false;
    if (arg.buf_id != file.buf_id) return false;
    if (arg.name != file.name) return false;
    if (arg.id != file.id) return false;
    if (arg.metadata != file.metadata) return false;

    return true;
}


void DbCheckRequestHandlerTests::ExpectRequestParams(asapo::Opcode op_code, const std::string& data_source,
        bool expect_compare) {

    std::string db_name = expected_beamtime_id;
    db_name += "_" + data_source;

    if (n_run  == 0) {
        EXPECT_CALL(mock_db, Connect_t(config.database_uri, db_name)).
        WillOnce(testing::Return(nullptr));

        EXPECT_CALL(*mock_request, GetDataSource())
        .WillRepeatedly(ReturnRef(data_source))
        ;
    }

    if (expect_compare) {
        EXPECT_CALL(*mock_request, GetDataSize())
        .WillRepeatedly(Return(expected_file_size))
        ;

        EXPECT_CALL(*mock_request, GetFileName())
        .WillRepeatedly(Return(expected_file_name))
        ;

        EXPECT_CALL(*mock_request, GetMetaData())
        .WillRepeatedly(ReturnRef(expected_metadata))
        ;
    }

    EXPECT_CALL(*mock_request, GetStream())
    .WillRepeatedly(Return(expected_stream))
    ;

    EXPECT_CALL(*mock_request, GetDataID())
    .WillRepeatedly(Return(expected_id))
    ;

    EXPECT_CALL(*mock_request, GetOpCode())
    .WillRepeatedly(Return(op_code))
    ;

    if (op_code == asapo::Opcode::kOpcodeTransferDatasetData) {
        EXPECT_CALL(*mock_request, GetCustomData_t())
        .WillRepeatedly(Return(expected_custom_data))
        ;
    }
}

MessageMeta DbCheckRequestHandlerTests::PrepareMessageMeta() {
    MessageMeta message_meta;
    message_meta.size = expected_file_size;
    message_meta.name = expected_file_name;
    message_meta.id = expected_id;
    message_meta.buf_id = expected_buf_id;
    message_meta.source = expected_host_uri;
    message_meta.metadata = expected_metadata;
    return message_meta;
}

void DbCheckRequestHandlerTests::MockGetByID(asapo::ErrorInterface* error, bool expect_compare ) {
    ExpectRequestParams(asapo::Opcode::kOpcodeTransferData, expected_data_source, expect_compare);
    EXPECT_CALL(mock_db, GetById_t(expected_collection_name, expected_id, _)).
    WillOnce(DoAll(
                 SetArgPointee<2>(expected_message_meta),
                 testing::Return(error)
             ));
}

void DbCheckRequestHandlerTests::MockGetSetByID(asapo::ErrorInterface* error, bool expect_compare ) {
    ExpectRequestParams(asapo::Opcode::kOpcodeTransferDatasetData, expected_data_source, expect_compare);
    EXPECT_CALL(mock_db, GetSetById_t(expected_collection_name, expected_dataset_id, expected_id, _)).
    WillOnce(DoAll(
                 SetArgPointee<3>(expected_message_meta),
                 testing::Return(error)
             ));
}


TEST_F(DbCheckRequestHandlerTests, ErrorIfRecordsDoNotMatch) {
    expected_message_meta.metadata = expected_metadata + "_";

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
    expected_message_meta.metadata = "{}";
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
        ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kInternalServerError));
        Mock::VerifyAndClearExpectations(mock_request.get());
    }
}

}
