#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockHttpClient.h"
#include "asapo/unittests/MockLogger.h"

#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/request_handler/request_handler.h"
#include "../../src/request_handler/request_handler_authorize.h"
#include "asapo/common/networking.h"
#include "../mock_receiver_config.h"
#include "asapo/preprocessor/definitions.h"

#include "../receiver_mocking.h"

#include "../../src/receiver_config.h"


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
using asapo::SourceCredentialsVersion;

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
    std::string expected_instance_id = "instance";
    std::string expected_pipeline_step = "pipestep";
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_data_source = "source";
    std::string expected_beamline = "beamline";
    std::string expected_beamline_path = "/beamline/p01/current";
    std::string expected_core_path = "/gpfs/blabla";
    std::string expected_producer_uri = "producer_uri";
    std::string expected_authorization_server = "authorizer_host";

    std::string expected_source_credentials_old_format;
    std::string expect_request_string_old_format;
    char const_old_format_message[1024]; // kMaxMessageSize

    std::string expected_source_credentials_new_format;
    std::string expect_request_string_new_format;
    char const_new_format_message[1024]; // kMaxMessageSize

    std::string expected_api_version = "v0.1";

    asapo::SourceType expected_source_type = asapo::SourceType::kProcessed;
    std::string expected_source_type_str = "processed";
    std::string expected_access_type_str = "[\"write\"]";

    void SetUp() override {
        expected_source_credentials_old_format = "processed%" + expected_beamtime_id + "%source%token";
        expect_request_string_old_format = std::string("{\"SourceCredentials\":\"") + expected_source_credentials_old_format +
                                "\",\"OriginHost\":\"" +
                                expected_producer_uri + "\"}";
        expected_source_credentials_new_format = "processed%" + expected_instance_id + "%" + expected_pipeline_step + "%" + expected_beamtime_id + "%source%token";
        expect_request_string_new_format = std::string("{\"SourceCredentials\":\"") + expected_source_credentials_new_format +
                                           "\",\"OriginHost\":\"" +
                                           expected_producer_uri + "\",\"NewFormat\": true}";

        strcpy(const_old_format_message, "");
        strcpy(const_new_format_message, "new_source_credentials_format");

        GenericRequestHeader request_header;
        mock_request.reset(new MockRequest{request_header, 1, expected_producer_uri, nullptr, nullptr});
        handler.http_client__ = std::unique_ptr<asapo::HttpClient> {&mock_http_client};
        handler.log__ = &mock_logger;
        config.authorization_server = expected_authorization_server;
        config.authorization_interval_ms = 0;
        SetReceiverConfig(config, "none");
    }
    void TearDown() override {
        handler.http_client__.release();
    }
    void MockAuthRequest(SourceCredentialsVersion format_type, bool error, HttpCode code = HttpCode::OK) {
        if (error) {

            if (format_type == SourceCredentialsVersion::NewVersion) {
                EXPECT_CALL(mock_http_client, Post_t(expected_authorization_server + "/authorize", _, expect_request_string_new_format, _, _)).
                WillOnce(
                        DoAll(SetArgPointee<4>(new asapo::SimpleError("http error")),
                              Return("")
                              ));
            } else {
                EXPECT_CALL(mock_http_client, Post_t(expected_authorization_server + "/authorize", _, expect_request_string_old_format, _, _)).
                WillOnce(
                        DoAll(SetArgPointee<4>(new asapo::SimpleError("http error")),
                              Return("")
                              ));
            }
            EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("failure authorizing"),
                                                 HasSubstr("http error"),
                                                 HasSubstr(expected_beamtime_id),
                                                 HasSubstr(expected_producer_uri),
                                                 HasSubstr(expected_authorization_server))));

        } else {
            if (format_type == SourceCredentialsVersion::NewVersion) {
                EXPECT_CALL(mock_http_client, Post_t(expected_authorization_server + "/authorize", _, expect_request_string_new_format, _, _)).
                WillOnce(
                        DoAll(SetArgPointee<4>(nullptr),
                              SetArgPointee<3>(code),
                              Return("{\"instanceId\":\"" + expected_instance_id + "\","
                                     "\"pipelineStep\":" + "\"" + expected_pipeline_step + "\","
                                     "\"beamtimeId\":" + "\"" + expected_beamtime_id + "\","
                                     "\"dataSource\":" + "\"" + expected_data_source + "\","
                                     "\"beamline-path\":" + "\"" + expected_beamline_path + "\","
                                     "\"corePath\":" + "\"" + expected_core_path + "\","
                                     "\"source-type\":" + "\"" + expected_source_type_str + "\","
                                     "\"beamline\":" + "\"" + expected_beamline + "\","
                                     "\"access-types\":" + expected_access_type_str + "}")
                              ));
                if (code != HttpCode::OK) {
                    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("failure authorizing"),
                                                         HasSubstr(expected_source_type_str),
                                                         HasSubstr(expected_instance_id),
                                                         HasSubstr(expected_pipeline_step),
                                                         HasSubstr(expected_beamtime_id),
                                                         HasSubstr(expected_data_source),
                                                         HasSubstr(expected_producer_uri),
                                                         HasSubstr(expected_authorization_server))));
                } else if (expected_access_type_str == "[\"write\"]") {
                    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("authorized"),
                                                         HasSubstr(expected_beamtime_id),
                                                         HasSubstr(expected_instance_id),
                                                         HasSubstr(expected_pipeline_step),
                                                         HasSubstr(expected_beamline),
                                                         HasSubstr(expected_source_type_str),
                                                         HasSubstr(expected_data_source),
                                                         HasSubstr(expected_producer_uri))));
                } else {
                    EXPECT_CALL(mock_logger, Error(HasSubstr("wrong")));
                }
            } else {
                EXPECT_CALL(mock_http_client, Post_t(expected_authorization_server + "/authorize", _, expect_request_string_old_format, _, _)).
                WillOnce(
                        DoAll(SetArgPointee<4>(nullptr),
                              SetArgPointee<3>(code),
                              Return("{\"instanceId\":\"" + std::string("Unset") + "\","
                                           "\"pipelineStep\": \"Unset\","
                                           "\"beamtimeId\":" + "\"" + expected_beamtime_id + "\","
                                           "\"dataSource\":" + "\"" + expected_data_source + "\","
                                           "\"beamline-path\":" + "\"" + expected_beamline_path + "\","
                                           "\"corePath\":" + "\"" + expected_core_path + "\","
                                           "\"source-type\":" + "\"" + expected_source_type_str + "\","
                                           "\"beamline\":" + "\"" + expected_beamline + "\","
                                           "\"access-types\":" + expected_access_type_str + "}")
                              ));
                if (code != HttpCode::OK) {
                    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("failure authorizing"),
                                                         HasSubstr(expected_source_type_str),
                                                         HasSubstr("Unset"), // For instance and pipelineStep
                                                         HasSubstr(expected_beamtime_id),
                                                         HasSubstr(expected_data_source),
                                                         HasSubstr(expected_producer_uri),
                                                         HasSubstr(expected_authorization_server))));
                } else if (expected_access_type_str == "[\"write\"]") {
                    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("authorized"),
                                                         HasSubstr("Unset"), // For instance and pipelineStep
                                                         HasSubstr(expected_beamtime_id),
                                                         HasSubstr(expected_beamline),
                                                         HasSubstr(expected_source_type_str),
                                                         HasSubstr(expected_data_source),
                                                         HasSubstr(expected_producer_uri))));
                } else {
                    EXPECT_CALL(mock_logger, Error(HasSubstr("wrong")));
                }
            }
        }


    }
    Error MockFirstAuthorization(SourceCredentialsVersion format_type, bool error, HttpCode code = HttpCode::OK) {
        EXPECT_CALL(*mock_request, GetOpCode())
        .WillOnce(Return(asapo::kOpcodeAuthorize));

        if (format_type == SourceCredentialsVersion::NewVersion) {
            EXPECT_CALL(*mock_request, GetMetaData())
            .WillOnce(ReturnRef(expected_source_credentials_new_format));

            EXPECT_CALL(*mock_request, GetMessage())
            .WillOnce(Return(const_new_format_message));
        } else {
            EXPECT_CALL(*mock_request, GetMetaData())
            .WillOnce(ReturnRef(expected_source_credentials_old_format));

            EXPECT_CALL(*mock_request, GetMessage())
            .WillOnce(Return(const_old_format_message));
        }

        EXPECT_CALL(*mock_request, GetApiVersion())
        .WillOnce(Return(expected_api_version));

        MockAuthRequest(format_type, error, code);
        return handler.ProcessRequest(mock_request.get());
    }
    Error MockRequestAuthorization(SourceCredentialsVersion format_type, bool error, HttpCode code = HttpCode::OK, bool set_request = true) {
        EXPECT_CALL(*mock_request, GetOpCode())
        .WillOnce(Return(asapo::kOpcodeTransferData));

        if (!error && code == HttpCode::OK && set_request) {
            EXPECT_CALL(*mock_request, SetSourceType(expected_source_type));
            if (format_type == SourceCredentialsVersion::NewVersion) {
                EXPECT_CALL(*mock_request, SetProducerInstanceId(expected_instance_id));
                EXPECT_CALL(*mock_request, SetPipelineStepId(expected_pipeline_step));
            }
            EXPECT_CALL(*mock_request, SetBeamtimeId(expected_beamtime_id));
            EXPECT_CALL(*mock_request, SetDataSource(expected_data_source));
            EXPECT_CALL(*mock_request, SetOfflinePath(expected_core_path));
            EXPECT_CALL(*mock_request, SetOnlinePath(expected_beamline_path));
            EXPECT_CALL(*mock_request, SetBeamline(expected_beamline));
        }

        MockAuthRequest(format_type, error, code);
        return handler.ProcessRequest(mock_request.get());
    }

};

// ---------
// Generic Format
// ---------

TEST_F(AuthorizerHandlerTests, CheckStatisticEntity) {
    auto entity = handler.GetStatisticEntity();
    ASSERT_THAT(entity, Eq(asapo::StatisticEntity::kNetworkIncoming));
}

TEST_F(AuthorizerHandlerTests, ErrorNotAuthorizedYet) {
    EXPECT_CALL(*mock_request, GetOpCode())
    .WillOnce(Return(asapo::kOpcodeTransferData))
    ;

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(AuthorizerHandlerTests, RequestFromUnsupportedClient) {
    EXPECT_CALL(*mock_request, GetOpCode())
    .WillOnce(Return(asapo::kOpcodeAuthorize))
    ;
    EXPECT_CALL(*mock_request, GetApiVersion())
    .WillOnce(Return("v1000.2"))
    ;

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kUnsupportedClient));
}

// ---------
// Old Format
// ---------

TEST_F(AuthorizerHandlerTests, OldFormat_ErrorProcessingAuthorizeRequest) {
    auto err = MockFirstAuthorization(SourceCredentialsVersion::OldVersion, true);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kInternalServerError));
}

TEST_F(AuthorizerHandlerTests, OldFormat_AuthorizeRequestreturns401) {

    auto err = MockFirstAuthorization(SourceCredentialsVersion::OldVersion, false, HttpCode::Unauthorized);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}


TEST_F(AuthorizerHandlerTests, OldFormat_AuthorizeOk) {
    auto err = MockFirstAuthorization(SourceCredentialsVersion::OldVersion, false);

    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(AuthorizerHandlerTests, OldFormat_AuthorizeFailsOnWrongAccessType) {
    expected_access_type_str = "[\"read\"]";
    auto err = MockFirstAuthorization(SourceCredentialsVersion::OldVersion, false);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(AuthorizerHandlerTests, OldFormat_ErrorOnSecondAuthorize) {
    MockFirstAuthorization(SourceCredentialsVersion::OldVersion, false);
    EXPECT_CALL(*mock_request, GetOpCode())
    .WillOnce(Return(asapo::kOpcodeAuthorize));

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("failure authorizing"),
                                         HasSubstr("already authorized"),
                                         HasSubstr(expected_authorization_server))));


    auto err =  handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(AuthorizerHandlerTests, OldFormat_ErrorOnDataTransferRequestAuthorize) {
    MockFirstAuthorization(SourceCredentialsVersion::OldVersion, false);

    auto err = MockRequestAuthorization(SourceCredentialsVersion::OldVersion, true);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kInternalServerError));
}


TEST_F(AuthorizerHandlerTests, OldFormat_DataTransferRequestAuthorizeReturns401) {
    MockFirstAuthorization(SourceCredentialsVersion::OldVersion, false);

    auto err = MockRequestAuthorization(SourceCredentialsVersion::OldVersion, false, HttpCode::Unauthorized);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kReAuthorizationFailure));
}

TEST_F(AuthorizerHandlerTests, OldFormat_DataTransferRequestAuthorizeReturnsSameBeamtimeId) {
    MockFirstAuthorization(SourceCredentialsVersion::OldVersion, false);
    auto err = MockRequestAuthorization(SourceCredentialsVersion::OldVersion, false);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(AuthorizerHandlerTests, OldFormat_RequestAuthorizeReturnsDifferentBeamtimeId) {
    MockFirstAuthorization(SourceCredentialsVersion::OldVersion, false);

    expected_beamtime_id = "different_id";
    auto err = MockRequestAuthorization(SourceCredentialsVersion::OldVersion, false, HttpCode::OK, false);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kReAuthorizationFailure));
}

TEST_F(AuthorizerHandlerTests, OldFormat_DataTransferRequestAuthorizeUsesCachedValue) {
    config.authorization_interval_ms = 10000;
    SetReceiverConfig(config, "none");
    MockFirstAuthorization(SourceCredentialsVersion::OldVersion, false);
    EXPECT_CALL(*mock_request, GetOpCode())
    .WillOnce(Return(asapo::kOpcodeTransferData));
    EXPECT_CALL(mock_http_client, Post_t(_, _, _, _, _)).Times(0);
    EXPECT_CALL(*mock_request, SetBeamtimeId(expected_beamtime_id));
    EXPECT_CALL(*mock_request, SetBeamline(expected_beamline));
    EXPECT_CALL(*mock_request, SetDataSource(expected_data_source));
    EXPECT_CALL(*mock_request, SetOnlinePath(expected_beamline_path));
    EXPECT_CALL(*mock_request, SetOfflinePath(expected_core_path));
    EXPECT_CALL(*mock_request, SetSourceType(expected_source_type));
    auto err =  handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(nullptr));
}

// ---------
// New Format
// ---------

TEST_F(AuthorizerHandlerTests, NewFormat_ErrorProcessingAuthorizeRequest) {
    auto err = MockFirstAuthorization(SourceCredentialsVersion::NewVersion, true);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kInternalServerError));
}

TEST_F(AuthorizerHandlerTests, NewFormat_AuthorizeRequestreturns401) {

    auto err = MockFirstAuthorization(SourceCredentialsVersion::NewVersion, false, HttpCode::Unauthorized);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}


TEST_F(AuthorizerHandlerTests, NewFormat_AuthorizeOk) {
    auto err = MockFirstAuthorization(SourceCredentialsVersion::NewVersion, false);

    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(AuthorizerHandlerTests, NewFormat_AuthorizeFailsOnWrongAccessType) {
    expected_access_type_str = "[\"read\"]";
    auto err = MockFirstAuthorization(SourceCredentialsVersion::NewVersion, false);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(AuthorizerHandlerTests, NewFormat_ErrorOnSecondAuthorize) {
    MockFirstAuthorization(SourceCredentialsVersion::NewVersion, false);
    EXPECT_CALL(*mock_request, GetOpCode())
    .WillOnce(Return(asapo::kOpcodeAuthorize));

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("failure authorizing"),
                                         HasSubstr("already authorized"),
                                         HasSubstr(expected_authorization_server))));


    auto err =  handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(AuthorizerHandlerTests, NewFormat_ErrorOnDataTransferRequestAuthorize) {
    MockFirstAuthorization(SourceCredentialsVersion::NewVersion, false);

    auto err = MockRequestAuthorization(SourceCredentialsVersion::NewVersion, true);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kInternalServerError));
}


TEST_F(AuthorizerHandlerTests, NewFormat_DataTransferRequestAuthorizeReturns401) {
    MockFirstAuthorization(SourceCredentialsVersion::NewVersion, false);

    auto err = MockRequestAuthorization(SourceCredentialsVersion::NewVersion, false, HttpCode::Unauthorized);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kReAuthorizationFailure));
}

TEST_F(AuthorizerHandlerTests, NewFormat_DataTransferRequestAuthorizeReturnsSameBeamtimeId) {
    MockFirstAuthorization(SourceCredentialsVersion::NewVersion, false);
    auto err = MockRequestAuthorization(SourceCredentialsVersion::NewVersion, false);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(AuthorizerHandlerTests, NewFormat_RequestAuthorizeReturnsDifferentBeamtimeId) {
    MockFirstAuthorization(SourceCredentialsVersion::NewVersion, false);

    expected_beamtime_id = "different_id";
    auto err = MockRequestAuthorization(SourceCredentialsVersion::NewVersion, false, HttpCode::OK, false);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kReAuthorizationFailure));
}

TEST_F(AuthorizerHandlerTests, Newormat_DataTransferRequestAuthorizeUsesCachedValue) {
    config.authorization_interval_ms = 10000;
    SetReceiverConfig(config, "none");
    MockFirstAuthorization(SourceCredentialsVersion::NewVersion, false);
    EXPECT_CALL(*mock_request, GetOpCode())
    .WillOnce(Return(asapo::kOpcodeTransferData));
    EXPECT_CALL(mock_http_client, Post_t(_, _, _, _, _)).Times(0);
    EXPECT_CALL(*mock_request, SetBeamtimeId(expected_beamtime_id));
    EXPECT_CALL(*mock_request, SetBeamline(expected_beamline));
    EXPECT_CALL(*mock_request, SetDataSource(expected_data_source));
    EXPECT_CALL(*mock_request, SetOnlinePath(expected_beamline_path));
    EXPECT_CALL(*mock_request, SetOfflinePath(expected_core_path));
    EXPECT_CALL(*mock_request, SetSourceType(expected_source_type));
    auto err =  handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(nullptr));
}

}
