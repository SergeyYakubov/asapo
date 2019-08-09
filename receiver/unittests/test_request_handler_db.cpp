#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockDatabase.h"
#include "unittests/MockLogger.h"

#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_factory.h"
#include "../src/request_handler.h"
#include "../src/request_handler_db.h"
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
using asapo::RequestHandlerDb;
using ::asapo::GenericRequestHeader;

using asapo::MockDatabase;
using asapo::RequestFactory;
using asapo::SetReceiverConfig;
using asapo::ReceiverConfig;


namespace {

class DbHandlerTests : public Test {
  public:
    std::string expected_collection_name = "test";
    RequestHandlerDb handler{expected_collection_name};
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<MockDatabase> mock_db;
    NiceMock<asapo::MockLogger> mock_logger;
    ReceiverConfig config;
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_stream = "stream";
    std::string expected_default_stream = "detector";

    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 2;
        handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
        handler.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, ""});
        ON_CALL(*mock_request, GetBeamtimeId()).WillByDefault(ReturnRef(expected_beamtime_id));
        ON_CALL(*mock_request, GetStream()).WillByDefault(ReturnRef(expected_stream));

    }
    void TearDown() override {
        handler.db_client__.release();
    }


};

TEST(DbHandler, Constructor) {
    RequestHandlerDb handler{""};
    ASSERT_THAT(dynamic_cast<asapo::MongoDBClient*>(handler.db_client__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(handler.log__), Ne(nullptr));
}


TEST_F(DbHandlerTests, CheckStatisticEntity) {
    auto entity = handler.GetStatisticEntity();
    ASSERT_THAT(entity, Eq(asapo::StatisticEntity::kDatabase));
}


TEST_F(DbHandlerTests, ProcessRequestCallsConnectDbWhenNotConnected) {
    config.broker_db_uri = "127.0.0.1:27017";
    SetReceiverConfig(config, "none");


    EXPECT_CALL(*mock_request, GetBeamtimeId())
    .WillOnce(ReturnRef(expected_beamtime_id))
    ;

    EXPECT_CALL(*mock_request, GetStream())
    .WillOnce(ReturnRef(expected_stream))
    ;



    EXPECT_CALL(mock_db, Connect_t("127.0.0.1:27017", expected_beamtime_id + "_" + expected_stream,
                                   expected_collection_name)).
    WillOnce(testing::Return(nullptr));

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(DbHandlerTests, ProcessRequestUsesCorrectDbNameForDetector) {
    config.broker_db_uri = "127.0.0.1:27017";
    SetReceiverConfig(config, "none");


    EXPECT_CALL(*mock_request, GetBeamtimeId())
    .WillOnce(ReturnRef(expected_beamtime_id))
    ;

    EXPECT_CALL(*mock_request, GetStream())
    .WillOnce(ReturnRef(expected_default_stream))
    ;


    EXPECT_CALL(mock_db, Connect_t("127.0.0.1:27017", expected_beamtime_id, expected_collection_name)).
    WillOnce(testing::Return(nullptr));

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(DbHandlerTests, ProcessRequestReturnsErrorWhenCannotConnect) {

    EXPECT_CALL(mock_db, Connect_t(_, _, expected_collection_name)).
    WillOnce(testing::Return(new asapo::SimpleError("")));

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Ne(nullptr));

}

TEST_F(DbHandlerTests, ProcessRequestDoesNotCallConnectSecondTime) {

    EXPECT_CALL(mock_db, Connect_t(_, _, expected_collection_name)).
    WillOnce(testing::Return(nullptr));

    handler.ProcessRequest(mock_request.get());
    handler.ProcessRequest(mock_request.get());
}


}