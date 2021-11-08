#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockDatabase.h"
#include "asapo/unittests/MockLogger.h"
#include "asapo/unittests/MockHttpClient.h"

#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/request_handler/request_factory.h"
#include "../../src/request_handler/request_handler.h"
#include "../../src/request_handler/request_handler_db.h"
#include "asapo/common/networking.h"
#include "../../../common/cpp/src/database/mongodb_client.h"
#include "../mock_receiver_config.h"
#include "asapo/common/data_structs.h"

#include "../receiver_mocking.h"

using namespace testing;
using namespace asapo;


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
    std::string expected_stream = "source";
    std::string expected_default_source = "detector";
    std::string expected_discovery_server = "discovery";
    std::string expected_database_server = "127.0.0.1:27017";

    MockHttpClient mock_http_client;

    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 2;
        handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
        handler.log__ = &mock_logger;
        handler.http_client__ = std::unique_ptr<asapo::HttpClient> {&mock_http_client};
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", nullptr});
        ON_CALL(*mock_request, GetBeamtimeId()).WillByDefault(ReturnRef(expected_beamtime_id));
        ON_CALL(*mock_request, GetDataSource()).WillByDefault(ReturnRef(expected_stream));

    }
    void TearDown() override {
        handler.http_client__.release();
        handler.db_client__.release();
    }
    void MockAuthRequest(bool error, HttpCode code = HttpCode::OK);

};


void DbHandlerTests::MockAuthRequest(bool error, HttpCode code) {
    if (error) {
        EXPECT_CALL(mock_http_client, Get_t(expected_discovery_server + "/asapo-mongodb",  _, _)).
        WillOnce(
            DoAll(SetArgPointee<2>(asapo::GeneralErrorTemplates::kSimpleError.Generate("http error").release()),
                  Return("")
                 ));
        EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("discovering database server"),
                                             HasSubstr("http error"),
                                             HasSubstr(expected_discovery_server))));

    } else {
        EXPECT_CALL(mock_http_client, Get_t(expected_discovery_server + "/asapo-mongodb", _, _)).
        WillOnce(
            DoAll(
                SetArgPointee<1>(code),
                SetArgPointee<2>(nullptr),
                Return(expected_database_server)
            ));
        if (code != HttpCode::OK) {
            EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("failure discover database server"),
                                                 HasSubstr("http code"),
                                                 HasSubstr(std::to_string(int(code))),
                                                 HasSubstr(expected_discovery_server))));
        } else {
            EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("discovered"),
                                                 HasSubstr(expected_database_server))));
        }
    }


}


TEST(DbHandler, Constructor) {
    RequestHandlerDb handler{""};
    ASSERT_THAT(dynamic_cast<asapo::MongoDBClient*>(handler.db_client__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(handler.log__), Ne(nullptr));
}


TEST_F(DbHandlerTests, CheckStatisticEntity) {
    auto entity = handler.GetStatisticEntity();
    ASSERT_THAT(entity, Eq(asapo::StatisticEntity::kDatabase));
}


TEST_F(DbHandlerTests, ProcessRequestDiscoversMongoDbAddress) {
    config.database_uri = "auto";
    config.discovery_server = expected_discovery_server;
    SetReceiverConfig(config, "none");

    MockAuthRequest(false, HttpCode::OK);

    EXPECT_CALL(*mock_request, GetBeamtimeId())
    .WillOnce(ReturnRef(expected_beamtime_id))
    ;

    EXPECT_CALL(*mock_request, GetDataSource())
    .WillOnce(ReturnRef(expected_stream))
    ;


    EXPECT_CALL(mock_db, Connect_t(expected_database_server, expected_beamtime_id + "_" + expected_stream)).
    WillOnce(testing::Return(nullptr));

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(DbHandlerTests, ProcessRequestErrorDiscoversMongoDbAddress) {
    config.database_uri = "auto";
    config.discovery_server = expected_discovery_server;
    SetReceiverConfig(config, "none");


    MockAuthRequest(true, HttpCode::BadRequest);

    EXPECT_CALL(mock_db, Connect_t(_, _)).Times(0);

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kInternalServerError));
}



TEST_F(DbHandlerTests, ProcessRequestCallsConnectDbWhenNotConnected) {
    config.database_uri = "127.0.0.1:27017";
    SetReceiverConfig(config, "none");


    EXPECT_CALL(*mock_request, GetBeamtimeId())
    .WillOnce(ReturnRef(expected_beamtime_id))
    ;

    EXPECT_CALL(*mock_request, GetDataSource())
    .WillOnce(ReturnRef(expected_stream))
    ;



    EXPECT_CALL(mock_db, Connect_t("127.0.0.1:27017", expected_beamtime_id + "_" + expected_stream)).
    WillOnce(testing::Return(nullptr));

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(DbHandlerTests, ProcessRequestReturnsErrorWhenCannotConnect) {

    EXPECT_CALL(mock_db, Connect_t(_, _)).
    WillOnce(testing::Return(asapo::GeneralErrorTemplates::kSimpleError.Generate().release()));

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kInternalServerError));

}

TEST_F(DbHandlerTests, ProcessRequestDoesNotCallConnectSecondTime) {

    EXPECT_CALL(mock_db, Connect_t(_, _)).
    WillOnce(testing::Return(nullptr));

    handler.ProcessRequest(mock_request.get());
    handler.ProcessRequest(mock_request.get());
}


}
