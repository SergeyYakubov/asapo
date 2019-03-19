#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockDatabase.h"
#include "unittests/MockLogger.h"

#include "../src/receiver_error.h"
#include "../src/request.h"
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
    RequestHandlerDbWrite handler;
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<MockDatabase> mock_db;
    NiceMock<asapo::MockLogger> mock_logger;
    ReceiverConfig config;
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_hostname = "host";
    uint64_t expected_port = 1234;
    uint64_t expected_buf_id = 18446744073709551615ull;
    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 2;
        handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
        handler.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, ""});
        ON_CALL(*mock_request, GetBeamtimeId()).WillByDefault(ReturnRef(expected_beamtime_id));
    }
    void TearDown() override {
        handler.db_client__.release();
    }


};

TEST(DBWritewr, Constructor) {
    RequestHandlerDbWrite handler;
    ASSERT_THAT(dynamic_cast<asapo::MongoDBClient*>(handler.db_client__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(handler.log__), Ne(nullptr));
}


TEST_F(DbWriterHandlerTests, CheckStatisticEntity) {
    auto entity = handler.GetStatisticEntity();
    ASSERT_THAT(entity, Eq(asapo::StatisticEntity::kDatabase));
}


TEST_F(DbWriterHandlerTests, ProcessRequestCallsConnectDbWhenNotConnected) {
    config.broker_db_uri = "127.0.0.1:27017";
    SetReceiverConfig(config, "none");


    EXPECT_CALL(*mock_request, GetBeamtimeId())
    .WillOnce(ReturnRef(expected_beamtime_id))
    ;


    EXPECT_CALL(mock_db, Connect_t("127.0.0.1:27017", expected_beamtime_id, asapo::kDBCollectionName)).
    WillOnce(testing::Return(nullptr));

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(DbWriterHandlerTests, ProcessRequestReturnsErrorWhenCannotConnect) {

    EXPECT_CALL(mock_db, Connect_t(_, _, asapo::kDBCollectionName)).
    WillOnce(testing::Return(new asapo::SimpleError("")));

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Ne(nullptr));

}

TEST_F(DbWriterHandlerTests, ProcessRequestDoesNotCallConnectSecondTime) {

    EXPECT_CALL(mock_db, Connect_t(_, _, asapo::kDBCollectionName)).
    WillOnce(testing::Return(nullptr));

    handler.ProcessRequest(mock_request.get());
    handler.ProcessRequest(mock_request.get());
}

MATCHER_P(CompareFileInfo, file, "") {
    if (arg.size != file.size) return false;
    if (arg.source != file.source) return false;
    if (arg.buf_id != file.buf_id) return false;
    if (arg.name != file.name) return false;
    if (arg.id != file.id) return false;

    return true;
}


TEST_F(DbWriterHandlerTests, CallsInsert) {
    config.broker_db_uri = "127.0.0.1:27017";
    config.source_host = expected_hostname;
    config.dataserver.listen_port = expected_port;

    SetReceiverConfig(config, "none");

    EXPECT_CALL(*mock_request, GetBeamtimeId())
    .WillOnce(ReturnRef(expected_beamtime_id))
    ;

    EXPECT_CALL(*mock_request, GetSlotId())
    .WillOnce(Return(expected_buf_id))
    ;


    EXPECT_CALL(mock_db, Connect_t(config.broker_db_uri, expected_beamtime_id, asapo::kDBCollectionName)).
    WillOnce(testing::Return(nullptr));

    std::string expected_file_name = "2";
    uint64_t expected_file_size = 10;
    uint64_t expected_id = 15;
    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(expected_file_size))
    ;

    EXPECT_CALL(*mock_request, GetFileName())
    .WillOnce(Return(expected_file_name))
    ;

    EXPECT_CALL(*mock_request, GetDataID())
    .WillOnce(Return(expected_id))
    ;

    FileInfo file_info;
    file_info.size = expected_file_size;
    file_info.name = expected_file_name;
    file_info.id = expected_id;
    file_info.buf_id = expected_buf_id;
    file_info.source = expected_hostname + ":" + std::to_string(expected_port);


    EXPECT_CALL(mock_db, Insert_t(CompareFileInfo(file_info), _)).
    WillOnce(testing::Return(nullptr));

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("insert record"),
                                         HasSubstr(config.broker_db_uri),
                                         HasSubstr(expected_beamtime_id),
                                         HasSubstr(asapo::kDBCollectionName)
                                        )
                                  )
               );

    handler.ProcessRequest(mock_request.get());
}

}