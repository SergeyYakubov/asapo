
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
using ::testing::ElementsAre;

using hidra2::JsonParser;
using hidra2::RapidJson;
using hidra2::MockIO;
using hidra2::IO;


namespace {

TEST(ParseString, SimpleConvertToJson) {
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

TEST(ParseString, EmbeddedConvertToJson) {
    std::string json = R"({"id":{"test":2}})";

    JsonParser parser{json, false};

    uint64_t id;
    auto err1 = parser.Embedded("id").GetUInt64("test", &id);

    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(id, Eq(2));
}

TEST(ParseString, DoubleEmbeddedConvertToJson) {
    std::string json = R"({"id":{"test":{"test2":2}}})";

    JsonParser parser{json, false};

    uint64_t id;
    auto err1 = parser.Embedded("id").Embedded("test").GetUInt64("test2", &id);

    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(id, Eq(2));
}

TEST(ParseString, ErrorOnWrongEmbeddedKey) {
    std::string json = R"({"id1":{"test":2}})";

    JsonParser parser{json, false};

    uint64_t id;
    auto err = parser.Embedded("id").GetUInt64("test", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("cannot find"));
}

TEST(ParseString, ErrorOnWrongEmbeddedSubKey) {
    std::string json = R"({"id1":{"test1":2}})";

    JsonParser parser{json, false};

    uint64_t id;
    auto err = parser.Embedded("id").GetUInt64("test", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("cannot find"));
}

TEST(ParseString, ErrorOnWrongKey) {
    std::string json = R"({"_id":"2"})";

    JsonParser parser{json, false};

    uint64_t id;
    auto err = parser.GetUInt64("_id1", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("cannot find"));
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


TEST(ParseString, IntArrayConvertToJson) {
    std::string json = R"({"array":[1,2,3]})";

    JsonParser parser{json, false};

    std::vector<uint64_t> vec;
    auto err = parser.GetArrayUInt64("array", &vec);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(vec, ElementsAre(1, 2, 3));
}

TEST(ParseString, IntArrayErrorConvertToJson) {
    std::string json = R"({"array":[1,2,"3"]})";

    JsonParser parser{json, false};

    std::vector<uint64_t> vec;
    auto err = parser.GetArrayUInt64("array", &vec);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), HasSubstr("type"));
}


TEST(ParseString, StringArrayConvertToJson) {
    std::string json = R"({"array":["s1","s2","s3"]})";

    JsonParser parser{json, false};

    std::vector<std::string> vec;
    auto err = parser.GetArrayString("array", &vec);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(vec, ElementsAre("s1", "s2", "s3"));
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
    WillOnce(DoAll(testing::SetArgPointee<1>(nullptr), testing::Return(json)));

    uint64_t id;
    auto err = parser.GetUInt64("_id", &id);
    ASSERT_THAT(id, Eq(2));
}
TEST_F(ParseFileTests, CannotReadFile) {
    std::string json = R"({"_id":2})";

    EXPECT_CALL(mock_io, ReadFileToString_t("filename", _)).
    WillOnce(DoAll(testing::SetArgPointee<1>(hidra2::IOErrorTemplate::kFileNotFound.Copy().release()),
                   testing::Return("")));

    uint64_t id;
    auto err = parser.GetUInt64("_id", &id);
    ASSERT_THAT(err->Explain(), HasSubstr(hidra2::IOErrorTemplate::kFileNotFound.Copy()->Explain()));


}

}
