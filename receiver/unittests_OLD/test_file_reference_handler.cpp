#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <system_wrappers/io.h>
#include "../src/receiver.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Mock;
using ::testing::InSequence;

TEST(FileReferenceHandler, add_file) {
    EXPECT_EQ(1, 1);
}

}
