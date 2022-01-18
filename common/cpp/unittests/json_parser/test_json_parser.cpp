
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include <chrono>

#include "asapo/json_parser/json_parser.h"
#include "../../src/json_parser/rapid_json.h"
#include "asapo/unittests/MockIO.h"


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
using ::testing::Pair;
using ::testing::DoAll;

using asapo::JsonFileParser;
using asapo::JsonStringParser;
using asapo::RapidJson;
using asapo::MockIO;
using asapo::IO;


namespace {

TEST(ParseString, SimpleConvertToJson) {
    std::string json = R"({"_id":2,"foo":"foo:\\1","bar":1,"flag":true})";

    JsonStringParser parser{json};

    uint64_t id, bar;
    std::string foo;
    bool flag;
    auto err1 = parser.GetUInt64("_id", &id);
    auto err2 = parser.GetString("foo", &foo);
    auto err3 = parser.GetUInt64("bar", &bar);
    auto err4 = parser.GetBool("flag", &flag);

    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(err2, Eq(nullptr));
    ASSERT_THAT(err3, Eq(nullptr));
    ASSERT_THAT(err4, Eq(nullptr));


    ASSERT_THAT(id, Eq(2));
    ASSERT_THAT(foo, Eq("foo:\\1"));
    ASSERT_THAT(bar, Eq(1));
    ASSERT_THAT(flag, true);

}

TEST(ParseString, EmbeddedConvertToJson) {
    std::string json = R"({"id":{"test":2}})";

    JsonStringParser parser{json};

    uint64_t id;
    auto err1 = parser.Embedded("id").GetUInt64("test", &id);

    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(id, Eq(2));
}

TEST(ParseString, DoubleEmbeddedConvertToJson) {
    std::string json = R"({"id":{"test":{"test2":2}}})";

    JsonStringParser parser{json};

    uint64_t id;
    auto err1 = parser.Embedded("id").Embedded("test").GetUInt64("test2", &id);

    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(id, Eq(2));
}

TEST(ParseString, ErrorOnWrongEmbeddedKey) {
    std::string json = R"({"id1":{"test":2}})";

    JsonStringParser parser{json};

    uint64_t id;
    auto err = parser.Embedded("id").GetUInt64("test", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("cannot find"));
}

TEST(ParseString, ErrorOnWrongEmbeddedSubKey) {
    std::string json = R"({"id1":{"test1":2}})";

    JsonStringParser parser{json};

    uint64_t id;
    auto err = parser.Embedded("id").GetUInt64("test", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("cannot find"));
}

TEST(ParseString, ErrorOnWrongKey) {
    std::string json = R"({"_id":"2"})";

    JsonStringParser parser{json};

    uint64_t id;
    auto err = parser.GetUInt64("_id1", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("cannot find"));
}

TEST(ParseString, ErrorOnWrongType) {
    std::string json = R"({"_id":"2"})";

    JsonStringParser parser{json};

    uint64_t id;
    auto err = parser.GetUInt64("_id", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("type"));

}

TEST(ParseString, ErrorOnWrongDocument) {
    std::string json = R"({"_id":2)";

    JsonStringParser parser{json};

    uint64_t id;
    auto err = parser.GetUInt64("_id", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("parse"));

}

TEST(ParseString, ErrorOnWrongFormatt) {
    std::string json = "10";

    JsonStringParser parser{json};

    uint64_t id;
    auto err = parser.GetUInt64("_id", &id);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), ::testing::HasSubstr("parse"));

}


TEST(ParseString, IntArrayConvertToJson) {
    std::string json = R"({"array":[1,2,3]})";

    JsonStringParser parser{json};

    std::vector<uint64_t> vec;
    auto err = parser.GetArrayUInt64("array", &vec);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(vec, ElementsAre(1, 2, 3));
}

TEST(ParseString, IntArrayConvertToJsonTwice) {
    std::string json = R"({"array":[1,2,3]})";

    JsonStringParser parser{json};

    std::vector<uint64_t> vec;
    auto err = parser.GetArrayUInt64("array", &vec);
    auto err2 = parser.GetArrayUInt64("array", &vec);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(err2, Eq(nullptr));
    ASSERT_THAT(vec, ElementsAre(1, 2, 3));
}


TEST(ParseString, IntArrayErrorConvertToJson) {
    std::string json = R"({"array":[1,2,"3"]})";

    JsonStringParser parser{json};

    std::vector<uint64_t> vec;
    auto err = parser.GetArrayUInt64("array", &vec);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(err->Explain(), HasSubstr("type"));
}



TEST(ParseString, StringArrayConvertToJson) {
    std::string json = R"({"array":["s1","s2","s3"]})";

    JsonStringParser parser{json};

    std::vector<std::string> vec;
    auto err = parser.GetArrayString("array", &vec);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(vec, ElementsAre("s1", "s2", "s3"));
}

TEST(ParseString, ObjectMemberArrayConvertToJson) {
    std::string json = R"({"object":{"k1":"v1","k2":"v2","k3":"v3"}})";

    JsonStringParser parser{json};

    std::vector<std::string> vec;
    auto err = parser.GetArrayObjectMembers("object", &vec);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(vec, ElementsAre("k1", "k2", "k3"));
}

TEST(ParseString, DictionaryStringConvertToJson) {
    std::string json = R"({"object":{"k1":"v1","k2":"v2","k3":"v3"}})";

    JsonStringParser parser{json};

    std::map<std::string, std::string> map;
    auto err = parser.GetDictionaryString("object", &map);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(map, ElementsAre(Pair("k1", "v1"), Pair("k2", "v2"), Pair("k3", "v3")));
}

TEST(ParseString, RawStringConvertToJson) {
    std::string json = R"({"object":{"k1":"v1","k2":"v2","k3":"v3"}})";

    JsonStringParser parser{json};

    std::string value;
    auto err = parser.GetRawString(&value);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(json, Eq(value));
}

TEST(ParseString, ArrayRawStringConvertToJson) {
    std::string json = R"({"array":[{"k1":"v1"},{"k2":"v2"},{"k3":"v3"}]})";

    JsonStringParser parser{json};

    std::vector<std::string> vec;
    auto err = parser.GetArrayRawStrings("array", &vec);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(vec, ElementsAre(R"({"k1":"v1"})", R"({"k2":"v2"})", R"({"k3":"v3"})"));
}

class ParseFileTests : public Test {
  public:
    NiceMock<MockIO> mock_io;
    std::unique_ptr<IO> io_ptr = std::unique_ptr<IO> {
        &mock_io
    };
    JsonFileParser parser{"filename", &io_ptr};

    void SetUp() override {
    }
    void TearDown() override {
        io_ptr.release();
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

TEST_F(ParseFileTests, InitializedOnlyOnce) {
    std::string json = R"({"_id":2})";

    EXPECT_CALL(mock_io, ReadFileToString_t("filename", _)).
    WillOnce(DoAll(testing::SetArgPointee<1>(nullptr), testing::Return(json)));

    uint64_t id, id2;
    auto err1 = parser.GetUInt64("_id", &id);
    auto err2 = parser.GetUInt64("_id", &id2);

    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(err2, Eq(nullptr));
    ASSERT_THAT(id, Eq(2));
    ASSERT_THAT(id2, Eq(2));
}


TEST_F(ParseFileTests, CannotReadFile) {
    std::string json = R"({"_id":2})";

    EXPECT_CALL(mock_io, ReadFileToString_t("filename", _)).
    WillOnce(DoAll(testing::SetArgPointee<1>(asapo::IOErrorTemplates::kFileNotFound.Generate().release()),
                   testing::Return("")));

    uint64_t id;
    auto err = parser.GetUInt64("_id", &id);
    ASSERT_THAT(err->Explain(), HasSubstr(asapo::IOErrorTemplates::kFileNotFound.Generate()->Explain()));


}



TEST_F(ParseFileTests, Flatten) {
    std::string json = R"({"top":"top","embedded":{"ar":[2,2,3],"str":"text"}})";
    std::string json_flat = R"({"meta.top":"top","meta.embedded.ar":[2,2,3],"meta.embedded.str":"text"})";
    JsonStringParser parser{json};

    std::string res;
    auto err = parser.GetFlattenedString("meta", ".", &res);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(res, Eq(json_flat));

}


TEST(ParseString, RawString) {
    std::string json = R"({"top":"top","embedded":{"ar":[2,2,3],"str":"text"}})";
    std::string json_row = R"({"ar":[2,2,3],"str":"text"})";
    JsonStringParser parser{json};

    std::string res;
    auto err = parser.Embedded("embedded").GetRawString(&res);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(res, Eq(json_row));

}


}
