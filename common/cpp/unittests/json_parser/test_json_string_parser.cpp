
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include <chrono>

#include "json_parser/json_parser.h"

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

using hidra2::JsonStringParser;

namespace {

TEST(ParseString, CorrectConvertToJson) {
    ASSERT_THAT(1, Eq(1));
    std::string json = R"({"_id":2,"foo":"foo","bar":1})";

    JsonStringParser parser{json};

    uint64_t id, bar;
    std::string foo;
    auto err1 = parser.GetUInt64("_id", &id);
    auto err2 = parser.GetString("foo", &foo);
    auto err3 = parser.GetUInt64("bar", &bar);

    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(err2, Eq(nullptr));
    ASSERT_THAT(err3, Eq(nullptr));

    ASSERT_THAT(id, Eq(2));
    ASSERT_THAT(foo, Eq("foo"));
    ASSERT_THAT(bar, Eq(1));

}


TEST(ParseString, ErrorOnWrongType) {
    ASSERT_THAT(1, Eq(1));
    std::string json = R"({"_id":"2"})";

    JsonStringParser parser{json};

    uint64_t id;
    auto err = parser.GetUInt64("_id", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("type"));

}

TEST(ParseString, ErrorOnWrongDocument) {
    ASSERT_THAT(1, Eq(1));
    std::string json = R"({"_id":2)";

    JsonStringParser parser{json};

    uint64_t id;
    auto err = parser.GetUInt64("_id", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("parse"));

}



}