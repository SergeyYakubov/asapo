#include <gmock/gmock.h>
#include <common/error.h>
#include "gtest/gtest.h"

using hidra2::Error;
using ::testing::Eq;
using ::testing::Ne;

namespace {

    TEST(ErrorTemplate, GenerateNoNullptr) {
        Error error = hidra2::ErrorTemplates::kEndOfFile.Generate();
        ASSERT_THAT(error, Ne(nullptr));
    }

    TEST(ErrorTemplate, EqCheck) {
        Error error = hidra2::ErrorTemplates::kEndOfFile.Generate();
        ASSERT_TRUE(hidra2::ErrorTemplates::kEndOfFile == error);
    }


    TEST(ErrorTemplate, NeCheck) {
        Error error = hidra2::ErrorTemplates::kEndOfFile.Generate();
        ASSERT_FALSE(hidra2::ErrorTemplates::kMemoryAllocationError == error);
    }
}
