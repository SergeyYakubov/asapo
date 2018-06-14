#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"
#include "../src/connection.h"
#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/statistics.h"
#include "mock_statistics.h"
#include "../src/connection_authorizer.h"
#include "../src/receiver_config.h"
#include "../src/receiver_config_factory.h"
#include "mock_receiver_config.h"


using ::testing::Test;
using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::InSequence;
using ::testing::HasSubstr;
using ::testing::StrEq;
using ::testing::SetArgPointee;
using ::testing::AllOf;
using testing::Sequence;


namespace {

//TEST(AuthorizerTests, AuthorizerReturnsOk) {
//    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("authorized"),HasSubstr("test"), HasSubstr(connected_uri))));
//}



}
