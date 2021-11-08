#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockHttpClient.h"
#include "asapo/unittests/MockLogger.h"

#include "../../src/receiver_error.h"
#include "asapo/common/networking.h"
#include "../mock_receiver_config.h"
#include "asapo/preprocessor/definitions.h"

#include "../receiver_mocking.h"

#include "../../src/receiver_config.h"
#include "../../src/request_handler/authorization_client.h"
#include "../../src/receiver_logger.h"


using namespace testing;
using namespace asapo;


namespace {

TEST(Authorizer, Constructor) {
    AuthorizationClient client;
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(client.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::HttpClient*>(client.http_client__.get()), Ne(nullptr));
}


class AuthorizerClientTests : public Test {
  public:
    AuthorizationClient client;
    AuthorizationData expected_auth_data;
    MockHttpClient mock_http_client;
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    ReceiverConfig config;

    NiceMock<asapo::MockLogger> mock_logger;
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_data_source = "source";
    std::string expected_beamline = "beamline";
    std::string expected_beamline_path = "/beamline/p01/current";
    std::string expected_core_path = "/gpfs/blabla";
    std::string expected_producer_uri = "producer_uri";
    std::string expected_authorization_server = "authorizer_host";
    std::string expect_request_string;
    std::string expected_source_credentials;
    asapo::SourceType expected_source_type = asapo::SourceType::kProcessed;

    std::string expected_source_type_str = "processed";
    std::string expected_access_type_str = "[\"write\"]";
    void SetUp() override {
        GenericRequestHeader request_header;
        expected_source_credentials = "processed%" + expected_beamtime_id + "%source%token";
        expected_auth_data.source_credentials = expected_source_credentials;
        expect_request_string = std::string("{\"SourceCredentials\":\"") + expected_source_credentials +
                                "\",\"OriginHost\":\"" +
                                expected_producer_uri + "\"}";

        mock_request.reset(new NiceMock<MockRequest>{request_header, 1, expected_producer_uri, nullptr});
        client.http_client__ = std::unique_ptr<asapo::HttpClient> {&mock_http_client};
        client.log__ = &mock_logger;
        config.authorization_server = expected_authorization_server;
        config.authorization_interval_ms = 0;
        SetReceiverConfig(config, "none");
        ON_CALL(*mock_request, GetOriginHost()).WillByDefault(ReturnRef(""));

    }
    void TearDown() override {
        client.http_client__.release();
    }
    void MockAuthRequest(bool error, HttpCode code = HttpCode::OK) {
        EXPECT_CALL(*mock_request,
                    GetOriginUri()).WillOnce(ReturnRef(expected_producer_uri));
        if (error) {
            EXPECT_CALL(mock_http_client,
                        Post_t(expected_authorization_server + "/authorize", _, expect_request_string, _, _)).
            WillOnce(
                DoAll(SetArgPointee<4>(asapo::GeneralErrorTemplates::kSimpleError.Generate("http error").release()),
                      Return("")
                     ));
        } else {
            EXPECT_CALL(mock_http_client,
                        Post_t(expected_authorization_server + "/authorize", _, expect_request_string, _, _)).
            WillOnce(
                DoAll(SetArgPointee<4>(nullptr),
                      SetArgPointee<3>(code),
                      Return("{\"beamtimeId\":\"" + expected_beamtime_id +
                             "\",\"dataSource\":" + "\"" + expected_data_source +
                             "\",\"beamline-path\":" + "\"" + expected_beamline_path +
                             "\",\"corePath\":" + "\"" + expected_core_path +
                             "\",\"source-type\":" + "\"" + expected_source_type_str +
                             "\",\"beamline\":" + "\"" + expected_beamline +
                             "\",\"access-types\":" + expected_access_type_str + "}")
                     ));
        }
    }
};


TEST_F(AuthorizerClientTests, AuthorizeRequestReturnsInternalerror) {
    MockAuthRequest(true);

    auto err = client.Authorize(mock_request.get(), &expected_auth_data);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kInternalServerError));
}

TEST_F(AuthorizerClientTests, AuthorizeRequestReturns401) {
    MockAuthRequest(false, HttpCode::Unauthorized);

    auto err = client.Authorize(mock_request.get(), &expected_auth_data);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}


TEST_F(AuthorizerClientTests, AuthorizeOk) {
    MockAuthRequest(false, HttpCode::OK);

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("authorized"),
                                         HasSubstr(expected_beamtime_id)
                                        )));

    auto err = client.Authorize(mock_request.get(), &expected_auth_data);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(expected_auth_data.data_source, Eq(expected_data_source));
    ASSERT_THAT(expected_auth_data.source_type, Eq(expected_source_type));
    ASSERT_THAT(expected_auth_data.beamline, Eq(expected_beamline));
    ASSERT_THAT(expected_auth_data.beamtime_id, Eq(expected_beamtime_id));
    ASSERT_THAT(expected_auth_data.offline_path, Eq(expected_core_path));
    ASSERT_THAT(expected_auth_data.online_path, Eq(expected_beamline_path));
    ASSERT_THAT(expected_auth_data.last_update, Gt(std::chrono::system_clock::time_point{}));
    ASSERT_THAT(expected_auth_data.source_credentials, Eq(expected_source_credentials));

}

TEST_F(AuthorizerClientTests, AuthorizeFailsOnWrongAccessType) {
    expected_access_type_str = "[\"read\"]";
    MockAuthRequest(false, HttpCode::OK);

    auto err = client.Authorize(mock_request.get(), &expected_auth_data);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(AuthorizerClientTests, AuthorizeFailsOnWrongAccessTypeForRaw) {
    expected_access_type_str = "[\"write\"]";
    expected_source_type_str = "raw";
    MockAuthRequest(false, HttpCode::OK);

    auto err = client.Authorize(mock_request.get(), &expected_auth_data);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(AuthorizerClientTests, AuthorizeFailsOnWrongAccessTypeForProcessed) {
    expected_access_type_str = "[\"writeraw\"]";
    expected_source_type_str = "processed";
    MockAuthRequest(false, HttpCode::OK);

    auto err = client.Authorize(mock_request.get(), &expected_auth_data);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(AuthorizerClientTests, AuthorizeOkForRaw) {
    expected_access_type_str = "[\"writeraw\"]";
    expected_source_type_str = "raw";
    expected_source_type = asapo::SourceType::kRaw;
    MockAuthRequest(false, HttpCode::OK);

    auto err = client.Authorize(mock_request.get(), &expected_auth_data);

    ASSERT_THAT(err, Eq(nullptr));
}

}
