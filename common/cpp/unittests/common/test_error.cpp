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

TEST(ErrorTemplate, Context) {
    Error error = asapo::GeneralErrorTemplates::kEndOfFile.Generate("test");
    error->AddContext("key", "value");
    error->AddContext("key2", "value2");

    ASSERT_THAT(error->Explain(), AllOf(HasSubstr("test"),
                                        HasSubstr("context"),
                                        HasSubstr("key:value"),
                                        HasSubstr("key2:value2")
                                       ));
}

TEST(ErrorTemplate, Cause) {
    Error error = asapo::GeneralErrorTemplates::kEndOfFile.Generate("test");
    Error error_c = asapo::GeneralErrorTemplates::kMemoryAllocationError.Generate("cause_test");
    Error error_c1 = asapo::GeneralErrorTemplates::kSimpleError.Generate("simple error");
    error->AddContext("key", "value");
    error_c->AddContext("key2", "value2");
    error_c->SetCause(std::move(error_c1));
    error->SetCause(std::move(error_c));
    ASSERT_THAT(error->Explain(), AllOf(HasSubstr("test"),
                                        HasSubstr("caused"),
                                        HasSubstr("key:value"),
                                        HasSubstr("key:value"),
                                        HasSubstr("key2:value2")
                                       ));
    ASSERT_THAT(error->ExplainPretty(), AllOf(HasSubstr("test"),
                                              HasSubstr("caused"),
                                              HasSubstr("key:value"),
                                              HasSubstr("key:value"),
                                              HasSubstr("key2:value2")
                                             ));
}

TEST(ErrorTemplate, Json) {
    Error error = asapo::GeneralErrorTemplates::kEndOfFile.Generate("test");
    Error error_c = asapo::GeneralErrorTemplates::kMemoryAllocationError.Generate("cause_test");
    error->AddContext("key", "value");
    error->SetCause(std::move(error_c));
    auto expected_string =
        R"("error":"end of file","message":"test","context":{"key":"value"},"cause":{"error":"memory allocation","message":"cause_test"})";
    ASSERT_THAT(error->ExplainInJSON(),  Eq(expected_string));
}



}
