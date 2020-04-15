#include <gmock/gmock.h>
#include <common/error.h>
#include "gtest/gtest.h"

using asapo::Error;
using ::testing::Eq;
using ::testing::Ne;

namespace {

TEST(ErrorTemplate, GenerateNoNullptr) {
    Error error = asapo::ErrorTemplates::kEndOfFile.Generate();
    ASSERT_THAT(error, Ne(nullptr));
}

TEST(ErrorTemplate, EqCheck) {
    Error error = asapo::ErrorTemplates::kEndOfFile.Generate();
    ASSERT_TRUE(asapo::ErrorTemplates::kEndOfFile == error);
}


TEST(ErrorTemplate, NeCheck) {
    Error error = asapo::ErrorTemplates::kEndOfFile.Generate();
    ASSERT_FALSE(asapo::ErrorTemplates::kMemoryAllocationError == error);
}
}
