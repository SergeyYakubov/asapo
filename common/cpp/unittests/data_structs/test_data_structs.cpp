#include "asapo/common/data_structs.h"
#include "asapo/preprocessor/definitions.h"
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include <chrono>


using asapo::MessageMeta;
using asapo::StreamInfo;
using asapo::SourceType;
using asapo::SourceCredentials;

using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;
using ::testing::_;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgPointee;


namespace {

uint64_t big_uint = 18446744073709551615ull;

MessageMeta PrepareMessageMeta() {
    MessageMeta message_meta;
    message_meta.size = 100;
    message_meta.id = 1;
    message_meta.name = std::string("folder") + asapo::kPathSeparator + "test";
    message_meta.source = "host:1234";
    message_meta.buf_id = big_uint;
    message_meta.timestamp = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(1));
    message_meta.metadata =  "{\"bla\":10}";
    return message_meta;
}

TEST(MessageMetaTests, Defaults) {
    MessageMeta message_meta;

    ASSERT_THAT(message_meta.buf_id, Eq(0));
    ASSERT_THAT(message_meta.id, Eq(0));
}


TEST(MessageMetaTests, CorrectConvertToJson) {
    auto message_meta = PrepareMessageMeta();
    std::string json = message_meta.Json();
    if (asapo::kPathSeparator == '/') {
        ASSERT_THAT(json, Eq(
                        R"({"_id":1,"size":100,"name":"folder/test","timestamp":1000000,"source":"host:1234","buf_id":-1,"meta":{"bla":10}})"));
    } else {
        ASSERT_THAT(json, Eq(
                        R"({"_id":1,"size":100,"name":"folder\\test","timestamp":1000000,"source":"host:1234","buf_id":-1,"meta":{"bla":10}})"));
    }
}

TEST(MessageMetaTests, CorrectConvertFromJsonReturnsError) {
    auto message_meta = PrepareMessageMeta();

    MessageMeta result;
    result.id = 10;

    std::string json = R"({"_id":2,"foo":"foo","bar":1})";

    auto ok = result.SetFromJson(json);

    ASSERT_THAT(ok, Eq(false));
    ASSERT_THAT(result.id, Eq(10));

}

TEST(MessageMetaTests, CorrectConvertFromJsonReturnsErrorForMetadata) {
    auto message_meta = PrepareMessageMeta();

    MessageMeta result;

    std::string json = R"({"_id":2,"foo":"foo","bar":1,{"meta":err}})";

    auto ok = result.SetFromJson(json);

    ASSERT_THAT(ok, Eq(false));

}



TEST(MessageMetaTests, CorrectConvertFromJson) {
    auto message_meta = PrepareMessageMeta();
    std::string json = message_meta.Json();

    MessageMeta result;
    auto ok = result.SetFromJson(json);

    ASSERT_THAT(ok, Eq(true));

    ASSERT_THAT(result.id, Eq(message_meta.id));
    ASSERT_THAT(result.name, Eq(message_meta.name));
    ASSERT_THAT(result.size, Eq(message_meta.size));
    ASSERT_THAT(result.timestamp, Eq(message_meta.timestamp));
    ASSERT_THAT(result.buf_id, Eq(message_meta.buf_id));
    ASSERT_THAT(result.source, Eq(message_meta.source));
    ASSERT_THAT(result.metadata, Eq(message_meta.metadata));

}


TEST(MessageMetaTests, CorrectConvertFromJsonEmptyMeta) {
    auto message_meta = PrepareMessageMeta();
    message_meta.metadata = "";
    std::string json = message_meta.Json();

    MessageMeta result;
    auto ok = result.SetFromJson(json);

    ASSERT_THAT(ok, Eq(true));
    ASSERT_THAT(result.metadata, Eq("{}"));

}


TEST(MessageMetaTests, EpochNanosecsFromNow) {
    auto ns = asapo::EpochNanosecsFromNow();
    ASSERT_THAT(ns, ::testing::Gt(0));
}


struct TestEpochFromISODate {
    std::string iso;
    uint64_t ns;
};


StreamInfo PrepareStreamInfo() {
    StreamInfo sinfo;
    sinfo.last_id = 123;
    sinfo.name = "test";
    sinfo.timestamp_created = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(1));
    sinfo.timestamp_lastentry = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(2));
    return sinfo;
}


TEST(MessageMetaTests, TimeFromNanosec) {
    auto tp = asapo::TimePointfromNanosec(1000);
    auto res = asapo::NanosecsEpochFromTimePoint(tp);
    ASSERT_THAT(res, Eq(1000));
}


TEST(StreamInfo, ConvertFromJson) {
    StreamInfo result;

    auto sinfo = PrepareStreamInfo();
    std::string json = sinfo.Json(true);

    auto ok = result.SetFromJson(json,true);

    ASSERT_THAT(ok, Eq(true));
    ASSERT_THAT(result.last_id, sinfo.last_id);
    ASSERT_THAT(result.name, sinfo.name);
    ASSERT_THAT(result.timestamp_created, sinfo.timestamp_created);
    ASSERT_THAT(result.timestamp_lastentry, sinfo.timestamp_lastentry);
}

TEST(StreamInfo, ConvertFromJsonWithoutID) {
    StreamInfo result;

    auto sinfo = PrepareStreamInfo();
    std::string json = sinfo.Json(false);

    auto ok = result.SetFromJson(json,false);

    ASSERT_THAT(ok, Eq(true));
    ASSERT_THAT(result.name, sinfo.name);
    ASSERT_THAT(result.timestamp_created, sinfo.timestamp_created);
}


TEST(StreamInfo, ConvertFromJsonErr) {
    StreamInfo result;

    std::string json = R"({"lastId":123)";
    auto ok = result.SetFromJson(json,true);

    ASSERT_THAT(ok, Eq(false));
    ASSERT_THAT(result.last_id, Eq(0));
}

TEST(StreamInfo, ConvertToJson) {
    auto sinfo = PrepareStreamInfo();

    std::string expected_json = R"({"lastId":123,"name":"test","timestampCreated":1000000,"timestampLast":2000000})";
    auto json = sinfo.Json(true);

    ASSERT_THAT(expected_json, Eq(json));
}

TEST(StreamInfo, ConvertToJsonWithoutID) {
    auto sinfo = PrepareStreamInfo();

    std::string expected_json = R"({"name":"test","timestampCreated":1000000})";
    auto json = sinfo.Json(false);

    ASSERT_THAT(expected_json, Eq(json));
}

TEST(SourceCredentials, ConvertToString) {
    auto sc = SourceCredentials{SourceType::kRaw,"beamtime","beamline","source","token"};
    std::string expected1= "raw%beamtime%beamline%source%token";
    std::string expected2= "processed%beamtime%beamline%source%token";

    auto res1 = sc.GetString();
    sc.type = asapo::SourceType::kProcessed;
    auto res2 = sc.GetString();

    ASSERT_THAT(res1, Eq(expected1));
    ASSERT_THAT(res2, Eq(expected2));
}

TEST(SourceCredentials, SourceTypeFromString) {
    SourceType type1,type2,type3;

    auto err1=GetSourceTypeFromString("raw",&type1);
    auto err2=GetSourceTypeFromString("processed",&type2);
    auto err3=GetSourceTypeFromString("bla",&type3);

    ASSERT_THAT(err1, Eq(nullptr));
    ASSERT_THAT(type1, Eq(SourceType::kRaw));
    ASSERT_THAT(err2, Eq(nullptr));
    ASSERT_THAT(type2, Eq(SourceType::kProcessed));
    ASSERT_THAT(err3, Ne(nullptr));
}

TEST(SourceCredentials, DefaultSourceTypeInSourceCreds) {
    SourceCredentials sc;

    ASSERT_THAT(sc.type, Eq(SourceType::kProcessed));
}
auto tests = std::vector<TestEpochFromISODate> {
    TestEpochFromISODate{"1970-01-01T00:00:00.0", 1}, // 0 reserved for errors
    TestEpochFromISODate{"1970-01-01", 1},
    TestEpochFromISODate{"1970-01-01T00:00:00.000000002", 2},
    TestEpochFromISODate{"2019-07-25T15:38:11.100010002", 1564069091100010002},
//errors
    TestEpochFromISODate{"1970-13-01T00:00:00.000000002", 0},
    TestEpochFromISODate{"1970-12-01T00:00:00.", 0},
};

TEST(MessageMetaTests, NanosecsEpochFromISODate) {
    for (auto test : tests) {
        auto res = asapo::NanosecsEpochFromISODate(test.iso);
        ASSERT_THAT(res, Eq(test.ns));
    }
}

auto tests2 = std::vector<TestEpochFromISODate> {
    TestEpochFromISODate{"1970-01-01T00:00:00", 0},
    TestEpochFromISODate{"1970-01-01T00:00:00.000000002", 2},
    TestEpochFromISODate{"2019-07-25T15:38:11.100010002", 1564069091100010002},
};

TEST(MessageMetaTests, ISODateFromNanosecsEpoch) {
    for (auto test : tests2) {
        auto res = asapo::IsoDateFromEpochNanosecs(test.ns);
        ASSERT_THAT(res, Eq(test.iso));
    }
}


}
