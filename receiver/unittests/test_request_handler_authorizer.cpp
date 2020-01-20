#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockHttpClient.h"
#include "unittests/MockLogger.h"

#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_handler.h"
#include "../src/request_handler_authorize.h"
#include "common/networking.h"
#include "mock_receiver_config.h"
#include "preprocessor/definitions.h"

#include "receiver_mocking.h"

#include "../src/receiver_config.h"


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

using asapo::MockRequest;
using ::asapo::Error;
using ::asapo::ErrorInterface;
using ::asapo::FileDescriptor;
using ::asapo::SocketDescriptor;
using ::asapo::MockHttpClient;
using asapo::Request;
using asapo::RequestHandlerAuthorize;
using ::asapo::GenericRequestHeader;
using asapo::ReceiverConfig;
using asapo::SetReceiverConfig;
using asapo::HttpCode;

namespace {

TEST(Authorizer, Constructor) {
    RequestHandlerAuthorize handler;
    ASSERT_THAT(dynamic_cast<asapo::HttpClient*>(handler.http_client__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(handler.log__), Ne(nullptr));
}


class AuthorizerHandlerTests : public Test {
  public:
    RequestHandlerAuthorize handler;
    MockHttpClient mock_http_client;
    std::unique_ptr<MockRequest> mock_request;
    ReceiverConfig config;

    NiceMock<asapo::MockLogger> mock_logger;
    std::string expected_source_credentials = "beamtime_id%stream%token";
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_stream = "stream";
    std::string expected_beamline = "beamline";
    std::string expected_facility = "facility";
    std::string expected_year = "2020";
    std::string expected_producer_uri = "producer_uri";
    std::string expected_authorization_server = "authorizer_host";
    std::string expect_request_string = std::string("{\"SourceCredentials\":\"") + expected_source_credentials +
                                        "\",\"OriginHost\":\"" +
                                        expected_producer_uri + "\"}";

    void MockRequestData();
    void SetUp() override {
        GenericRequestHeader request_header;
        mock_request.reset(new MockRequest{request_header, 1, expected_producer_uri});
        handler.http_client__ = std::unique_ptr<asapo::HttpClient> {&mock_http_client};
        handler.log__ = &mock_logger;
        config.authorization_server = expected_authorization_server;
        config.authorization_interval_ms = 0;
        SetReceiverConfig(config, "none");
    }
    void TearDown() override {
        handler.http_client__.release();
    }
    void MockAuthRequest(bool error, HttpCode code = HttpCode::OK) {
        if (error) {
            EXPECT_CALL(mock_http_client, Post_t(expected_authorization_server + "/authorize", expect_request_string, _, _)).
            WillOnce(
                DoAll(SetArgPointee<3>(new asapo::SimpleError("http error")),
                      Return("")
                     ));
            EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("failure authorizing"),
                                                 HasSubstr("http error"),
                                                 HasSubstr(expected_beamtime_id),
                                                 HasSubstr(expected_producer_uri),
                                                 HasSubstr(expected_authorization_server))));

        } else {
            EXPECT_CALL(mock_http_client, Post_t(expected_authorization_server + "/authorize", expect_request_string, _, _)).
            WillOnce(
                DoAll(SetArgPointee<3>(nullptr),
                      SetArgPointee<2>(code),
                      Return("{\"BeamtimeId\":\"" + expected_beamtime_id +
                             "\",\"Stream\":" + "\"" + expected_stream +
                             "\",\"Facility\":" + "\"" + expected_facility +
                             "\",\"Year\":" + "\"" + expected_year +
                             "\",\"Beamline\":" + "\"" + expected_beamline + "\"}")
                     ));
            if (code != HttpCode::OK) {
                EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("failure authorizing"),
                                                     HasSubstr("return code"),
                                                     HasSubstr(std::to_string(int(code))),
                                                     HasSubstr(expected_beamtime_id),
                                                     HasSubstr(expected_stream),
                                                     HasSubstr(expected_producer_uri),
                                                     HasSubstr(expected_authorization_server))));
            } else {
                EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("authorized"),
                                                     HasSubstr(expected_beamtime_id),
                                                     HasSubstr(expected_beamline),
                                                     HasSubstr(expected_stream),
                                                     HasSubstr(expected_producer_uri))));
            }
        }


    }
    Error MockFirstAuthorization(bool error, HttpCode code = HttpCode::OK) {
        EXPECT_CALL(*mock_request, GetOpCode())
        .WillOnce(Return(asapo::kOpcodeAuthorize))
        ;
        EXPECT_CALL(*mock_request, GetMessage())
        .WillOnce(Return(expected_source_credentials.c_str()))
        ;

        MockAuthRequest(error, code);
        return handler.ProcessRequest(mock_request.get());
    }
    Error MockRequestAuthorization(bool error, HttpCode code = HttpCode::OK) {
        EXPECT_CALL(*mock_request, GetOpCode())
        .WillOnce(Return(asapo::kOpcodeTransferData))
        ;
        if (!error && code == HttpCode::OK) {
            EXPECT_CALL(*mock_request, SetBeamtimeId(expected_beamtime_id));
            EXPECT_CALL(*mock_request, SetStream(expected_stream));
            EXPECT_CALL(*mock_request, SetFacility(expected_facility));
            EXPECT_CALL(*mock_request, SetBeamtimeYear(expected_year));
            EXPECT_CALL(*mock_request, SetBeamline(expected_beamline));
        }

        MockAuthRequest(error, code);
        return handler.ProcessRequest(mock_request.get());
    }

};

TEST_F(AuthorizerHandlerTests, CheckStatisticEntity) {
    auto entity = handler.GetStatisticEntity();
    ASSERT_THAT(entity, Eq(asapo::StatisticEntity::kNetwork));
}

TEST_F(AuthorizerHandlerTests, ErrorNotAuthorizedYet) {
    EXPECT_CALL(*mock_request, GetOpCode())
    .WillOnce(Return(asapo::kOpcodeTransferData))
    ;

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(AuthorizerHandlerTests, ErrorProcessingAuthorizeRequest) {

    auto err = MockFirstAuthorization(true);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kInternalServerError));
}


TEST_F(AuthorizerHandlerTests, AuthorizeRequestreturns401) {

    auto err = MockFirstAuthorization(false, HttpCode::Unauthorized);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}


TEST_F(AuthorizerHandlerTests, AuthorizeOk) {
    auto err = MockFirstAuthorization(false);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(AuthorizerHandlerTests, ErrorOnSecondAuthorize) {
    MockFirstAuthorization(false);
    EXPECT_CALL(*mock_request, GetOpCode())
    .WillOnce(Return(asapo::kOpcodeAuthorize));

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("failure authorizing"),
                                         HasSubstr("already authorized"),
                                         HasSubstr(expected_authorization_server))));


    auto err =  handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(AuthorizerHandlerTests, ErrorOnDataTransferRequestAuthorize) {
    MockFirstAuthorization(false);
    auto err = MockRequestAuthorization(true);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kInternalServerError));
}


TEST_F(AuthorizerHandlerTests, DataTransferRequestAuthorizeReturns401) {
    MockFirstAuthorization(false);

    auto err = MockRequestAuthorization(false, HttpCode::Unauthorized);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(AuthorizerHandlerTests, DataTransferRequestAuthorizeReturnsOK) {
    MockFirstAuthorization(false);

    auto err = MockRequestAuthorization(false);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(AuthorizerHandlerTests, DataTransferRequestAuthorizeUsesCachedValue) {
    config.authorization_interval_ms = 10000;
    SetReceiverConfig(config, "none");
    MockFirstAuthorization(false);
    EXPECT_CALL(*mock_request, GetOpCode())
    .WillOnce(Return(asapo::kOpcodeTransferData));
    EXPECT_CALL(mock_http_client, Post_t(_, _, _, _)).Times(0);
    EXPECT_CALL(*mock_request, SetBeamtimeId(expected_beamtime_id));
    EXPECT_CALL(*mock_request, SetBeamline(expected_beamline));
    EXPECT_CALL(*mock_request, SetStream(expected_stream));
    EXPECT_CALL(*mock_request, SetFacility(expected_facility));
    EXPECT_CALL(*mock_request, SetBeamtimeYear(expected_year));

    auto err =  handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(nullptr));
}




}