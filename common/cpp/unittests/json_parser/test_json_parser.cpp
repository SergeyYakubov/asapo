
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include <chrono>

#include "json_parser/json_parser.h"
#include "../../src/json_parser/rapid_json.h"
#include "unittests/MockIO.h"


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

using hidra2::JsonParser;
using hidra2::RapidJson;
using hidra2::MockIO;
using hidra2::IO;


namespace {

TEST(ParseString, CorrectConvertToJson) {
    std::string json = R"({"_id":2,"foo":"foo","bar":1})";

    JsonParser parser{json, false};

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
    std::string json = R"({"_id":"2"})";

    JsonParser parser{json, false};

    uint64_t id;
    auto err = parser.GetUInt64("_id", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("type"));

}

TEST(ParseString, ErrorOnWrongDocument) {
    std::string json = R"({"_id":2)";

    JsonParser parser{json, false};

    uint64_t id;
    auto err = parser.GetUInt64("_id", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("parse"));

}

class ParseFileTests : public Test {
  public:
    RapidJson parser{"filename", true};
    NiceMock<MockIO> mock_io;
    void SetUp() override {
        parser.io__ = std::unique_ptr<IO> {&mock_io};
    }
    void TearDown() override {
        parser.io__.release();
    }
};

TEST_F(ParseFileTests, CorrectConvertFileToJson) {
    std::string json = R"({"_id":2})";

    EXPECT_CALL(mock_io, ReadFileToString_t("filename", _)).
    WillOnce(DoAll(testing::SetArgPointee<1>(static_cast<hidra2::SimpleError*>(nullptr)), testing::Return(json)));

    uint64_t id;
    auto err = parser.GetUInt64("_id", &id);
    ASSERT_THAT(id, Eq(2));
}


TEST_F(ParseFileTests, CannotReadFile) {
    std::string json = R"({"_id":2})";

    EXPECT_CALL(mock_io, ReadFileToString_t("filename", _)).
    WillOnce(DoAll(testing::SetArgPointee<1>(new hidra2::SimpleError(hidra2::IOErrors::kFileNotFound)),
                   testing::Return("")));

    uint64_t id;
    auto err = parser.GetUInt64("_id", &id);
    ASSERT_THAT(err->Explain(), HasSubstr(hidra2::IOErrors::kFileNotFound));


}


}