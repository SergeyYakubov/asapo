#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../src/rds_response_error.h"

using namespace asapo;
using ::testing::Eq;

TEST(ConvertRdsResponseToError, TestAllCases) {
    ASSERT_THAT(ConvertRdsResponseToError(NetworkErrorCode::kNetErrorNoError /* 0 */), Eq(nullptr));
    ASSERT_THAT(ConvertRdsResponseToError(NetworkErrorCode::kNetErrorReauthorize),
                Eq(RdsResponseErrorTemplates::kNetErrorReauthorize));
    ASSERT_THAT(ConvertRdsResponseToError(NetworkErrorCode::kNetErrorWarning),
                Eq(RdsResponseErrorTemplates::kNetErrorWarning));
    ASSERT_THAT(ConvertRdsResponseToError(NetworkErrorCode::kNetErrorWrongRequest),
                Eq(RdsResponseErrorTemplates::kNetErrorWrongRequest));
    ASSERT_THAT(ConvertRdsResponseToError(NetworkErrorCode::kNetErrorNoData),
                Eq(RdsResponseErrorTemplates::kNetErrorNoData));
    ASSERT_THAT(ConvertRdsResponseToError(NetworkErrorCode::kNetAuthorizationError),
                Eq(RdsResponseErrorTemplates::kNetAuthorizationError));
    ASSERT_THAT(ConvertRdsResponseToError(NetworkErrorCode::kNetErrorInternalServerError),
                Eq(RdsResponseErrorTemplates::kNetErrorInternalServerError));
}
