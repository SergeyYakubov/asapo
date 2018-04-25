#include <gmock/gmock.h>
#include "gtest/gtest.h"

//#include "json_parser/json_parser.h"

#include "../../src/logger/spd_logger.h"

using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;
using ::testing::_;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::HasSubstr;
using ::testing::ElementsAre;


namespace {

TEST(DefaultLogger, BinLogger) {
    auto logger = hidra2::CreateDefaultLoggerBin("test");
    ASSERT_THAT(dynamic_cast<hidra2::SpdLogger*>(logger.get()), Ne(nullptr));
}

TEST(DefaultLogger, ApiLogger) {
    auto logger = hidra2::CreateDefaultLoggerApi("test","endpoint");
    ASSERT_THAT(dynamic_cast<hidra2::SpdLogger*>(logger.get()), Ne(nullptr));
}

}
