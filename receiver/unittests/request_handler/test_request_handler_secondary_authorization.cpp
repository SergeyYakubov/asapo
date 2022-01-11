#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <chrono>

#include "asapo/common/internal/version.h"
#include "asapo/common/networking.h"
#include "asapo/common/data_structs.h"


#include "../../src/request_handler/request_handler_secondary_authorization.h"
#include "../../src/request.h"
#include "../receiver_mocking.h"
#include "../mock_receiver_config.h"
#include "../../src/receiver_config.h"

using namespace testing;
using namespace asapo;

namespace {

class SecondaryAuthorizationHandlerTests : public Test {
  public:
    AuthorizationData auth_data;
    ReceiverConfig config;
    AuthorizationData* expected_auth_data{&auth_data};
    RequestHandlerSecondaryAuthorization handler{&auth_data};
    asapo::MockAuthorizationClient mock_authorization_client;
    std::unique_ptr<NiceMock<MockRequest>> mock_request;

    std::string expected_source_credentials = "source_creds";
    std::string expected_instance_id = "expected_instance_id";
    std::string expected_pipeline_step_id = "expected_pipeline_step_id";
    std::string expected_beamtime_id = "expected_beamtime_id";
    std::string expected_beamline = "expected_beamline";
    std::string expected_data_source = "expected_data_source";
    std::string expected_beamline_path = "expected_beamline_path";
    std::string expected_core_path = "expected_core_path";
    SourceType expected_source_type = SourceType::kProcessed;

    void SetUp() override {
        auth_data.data_source = expected_data_source;
        auth_data.source_credentials = expected_data_source;
        auth_data.source_type = expected_source_type;
        auth_data.beamtime_id = expected_beamtime_id;
        auth_data.last_update = std::chrono::system_clock::now();
        auth_data.online_path = expected_beamline_path;
        auth_data.offline_path = expected_core_path;
        auth_data.beamline = expected_beamline;
        auth_data.producer_instance_id = expected_instance_id;
        auth_data.pipeline_step_id = expected_pipeline_step_id;

        auth_data.source_credentials = expected_source_credentials;
        GenericRequestHeader request_header;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", nullptr, nullptr});
        handler.auth_client__ = std::unique_ptr<asapo::AuthorizationClient> {&mock_authorization_client};
        config.authorization_interval_ms = 10000;
        SetReceiverConfig(config, "none");
        ON_CALL(*mock_request, GetApiVersion()).WillByDefault(Return(asapo::GetReceiverApiVersion()));
        SetDefaultRequestCalls(mock_request.get(),expected_beamtime_id);

    }
    void ExpectAuth();
    void ExpectSetRequest() {
        EXPECT_CALL(*mock_request, SetProducerInstanceId(expected_instance_id));
        EXPECT_CALL(*mock_request, SetPipelineStepId(expected_pipeline_step_id));
        EXPECT_CALL(*mock_request, SetBeamtimeId(expected_beamtime_id));
        EXPECT_CALL(*mock_request, SetBeamline(expected_beamline));
        EXPECT_CALL(*mock_request, SetDataSource(expected_data_source));
        EXPECT_CALL(*mock_request, SetOnlinePath(expected_beamline_path));
        EXPECT_CALL(*mock_request, SetOfflinePath(expected_core_path));
        EXPECT_CALL(*mock_request, SetSourceType(expected_source_type));
    }
    void TearDown() override {
        handler.auth_client__.release();
    }
};

ACTION_P(A_WriteCache, beamtime_id) {
    ((AuthorizationData*)arg1)->beamtime_id = static_cast<std::string>(beamtime_id);
}

void SecondaryAuthorizationHandlerTests::ExpectAuth() {
    EXPECT_CALL(mock_authorization_client,
                Authorize_t(mock_request.get(), expected_auth_data)).WillOnce(DoAll(
                            A_WriteCache(expected_beamtime_id),
                            Return(nullptr)
                        ));
}


TEST_F(SecondaryAuthorizationHandlerTests, AuthorizeWithCacheOk) {
    ExpectSetRequest();

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(expected_auth_data->source_credentials, Eq(expected_source_credentials));
    ASSERT_THAT(expected_auth_data->beamtime_id, Eq(expected_beamtime_id));
}

TEST_F(SecondaryAuthorizationHandlerTests, ReauthorizeOk) {
    ExpectAuth();
    ExpectSetRequest();
    auth_data.last_update = std::chrono::system_clock::time_point{};

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(expected_auth_data->source_credentials, Eq(expected_source_credentials));
    ASSERT_THAT(expected_auth_data->beamtime_id, Eq(expected_beamtime_id));
}

TEST_F(SecondaryAuthorizationHandlerTests, ReauthorizeWrongBeamtime) {
    expected_beamtime_id = "new_beamtime";
    ExpectAuth();
    auth_data.last_update = std::chrono::system_clock::time_point{};

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kReAuthorizationFailure));
    ASSERT_THAT(expected_auth_data->source_credentials, Eq(""));
}

TEST_F(SecondaryAuthorizationHandlerTests, ReauthorizeWithAutherror) {
    EXPECT_CALL(mock_authorization_client, Authorize_t(_, _)).WillOnce(Return(
                ReceiverErrorTemplates::kAuthorizationFailure.Generate().release()));
    auth_data.last_update = std::chrono::system_clock::time_point{};

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kReAuthorizationFailure));
    ASSERT_THAT(expected_auth_data->source_credentials, Eq(""));
}


TEST_F(SecondaryAuthorizationHandlerTests, ErrorOnEmptyCache) {
    auth_data.source_credentials = "";

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(SecondaryAuthorizationHandlerTests, RequestFromUnsupportedClient) {
    EXPECT_CALL(*mock_request, GetApiVersion())
    .WillOnce(Return("v1000.2"));

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kUnsupportedClient));
}

}
