#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/common/internal/version.h"
#include "asapo/common/networking.h"

#include "../../src/request_handler/request_handler_initial_authorization.h"
#include "../../src/request.h"
#include "../receiver_mocking.h"

using namespace testing;
using namespace asapo;

namespace {

class InitialAuthorizationHandlerTests : public Test {
  public:
    AuthorizationData auth_data;
    AuthorizationData* expected_auth_data{&auth_data};
    RequestHandlerInitialAuthorization handler{&auth_data};
    asapo::MockAuthorizationClient mock_authorization_client;
    std::unique_ptr<MockRequest> mock_request;

    std::string expected_source_credentials = "source_creds";

    void SetUp() override {
        GenericRequestHeader request_header;
        mock_request.reset(new MockRequest{request_header, 1, "", nullptr});
        handler.auth_client__ = std::unique_ptr<asapo::AuthorizationClient> {&mock_authorization_client};
        SetDefaultRequestCalls(mock_request.get(),"");

    }
    void ExpectAuthMocks() {
        EXPECT_CALL(*mock_request, GetApiVersion()).WillRepeatedly(Return(asapo::GetReceiverApiVersion()));
        EXPECT_CALL(*mock_request, GetMetaData())
        .WillOnce(ReturnRef(expected_source_credentials))
        ;
        EXPECT_CALL(mock_authorization_client,
                    Authorize_t(mock_request.get(), expected_auth_data)).WillOnce(Return(nullptr));
    }
    void TearDown() override {
        handler.auth_client__.release();
    }
};

TEST_F(InitialAuthorizationHandlerTests, AuthorizeOk) {
    ExpectAuthMocks();
    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(expected_auth_data->source_credentials, Eq(expected_source_credentials));

}

TEST_F(InitialAuthorizationHandlerTests, ErrorOnSecondAuthorize) {
    ExpectAuthMocks();

    handler.ProcessRequest(mock_request.get());
    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
}

TEST_F(InitialAuthorizationHandlerTests, RequestFromUnsupportedClient) {
    EXPECT_CALL(*mock_request, GetApiVersion())
    .WillOnce(Return("v1000.2"));

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kUnsupportedClient));
}

TEST_F(InitialAuthorizationHandlerTests, CheckStatisticEntity) {
    auto entity = handler.GetStatisticEntity();
    ASSERT_THAT(entity, Eq(asapo::StatisticEntity::kNetwork));
}


}
