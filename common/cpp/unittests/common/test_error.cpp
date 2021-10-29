#include <gmock/gmock.h>
#include <asapo/common/error.h>
#include "gtest/gtest.h"

using asapo::Error;
using namespace testing;

namespace {

TEST(ErrorTemplate, GenerateNoNullptr) {
    Error error = asapo::GeneralErrorTemplates::kEndOfFile.Generate();
    ASSERT_THAT(error, Ne(nullptr));
}

TEST(ErrorTemplate, EqCheck) {
    Error error = asapo::GeneralErrorTemplates::kEndOfFile.Generate();
    ASSERT_TRUE(asapo::GeneralErrorTemplates::kEndOfFile == error);
}


TEST(ErrorTemplate, NeCheck) {
    Error error = asapo::GeneralErrorTemplates::kEndOfFile.Generate();
    ASSERT_FALSE(asapo::GeneralErrorTemplates::kMemoryAllocationError == error);
}

TEST(ErrorTemplate, Explain) {
    Error error = asapo::GeneralErrorTemplates::kEndOfFile.Generate("test");
    ASSERT_THAT(error->Explain(), HasSubstr("test"));
}

TEST(ErrorTemplate, Append) {
    Error error = asapo::GeneralErrorTemplates::kEndOfFile.Generate("test");
    ASSERT_THAT(error->Explain(), HasSubstr("test"));
}

}
