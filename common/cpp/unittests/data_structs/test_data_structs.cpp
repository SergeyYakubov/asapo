#include "common/data_structs.h"
#include "preprocessor/definitions.h"
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include <chrono>


using asapo::FileInfo;
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

FileInfo PrepareFileInfo() {
    FileInfo finfo;
    finfo.size = 100;
    finfo.id = 1;
    finfo.name = std::string("folder") + asapo::kPathSeparator + "test";
    finfo.source = "host:1234";
    finfo.buf_id = big_uint;
    finfo.timestamp = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(1));
    finfo.metadata =  "{\"bla\":10}";
    return finfo;
}

TEST(FileInFo, Defaults) {
    FileInfo finfo;

    ASSERT_THAT(finfo.buf_id, Eq(0));
    ASSERT_THAT(finfo.id, Eq(0));
}


TEST(FileInFo, CorrectConvertToJson) {
    auto finfo = PrepareFileInfo();
    std::string json = finfo.Json();
    if (asapo::kPathSeparator == '/') {
        ASSERT_THAT(json, Eq(
                        R"({"_id":1,"size":100,"name":"folder/test","timestamp":1000000,"source":"host:1234","buf_id":-1,"meta":{"bla":10}})"));
    } else {
        ASSERT_THAT(json, Eq(
                        R"({"_id":1,"size":100,"name":"folder\\test","timestamp":1000000,"source":"host:1234","buf_id":-1,"meta":{"bla":10}})"));
    }
}

TEST(FileInFo, CorrectConvertFromJsonReturnsError) {
    auto finfo = PrepareFileInfo();

    FileInfo result;
    result.id = 10;

    std::string json = R"({"_id":2,"foo":"foo","bar":1})";

    auto ok = result.SetFromJson(json);

    ASSERT_THAT(ok, Eq(false));
    ASSERT_THAT(result.id, Eq(10));

}

TEST(FileInFo, CorrectConvertFromJsonReturnsErrorForMetadata) {
    auto finfo = PrepareFileInfo();

    FileInfo result;

    std::string json = R"({"_id":2,"foo":"foo","bar":1,{"meta":err}})";

    auto ok = result.SetFromJson(json);

    ASSERT_THAT(ok, Eq(false));

}



TEST(FileInFo, CorrectConvertFromJson) {
    auto finfo = PrepareFileInfo();
    std::string json = finfo.Json();

    FileInfo result;
    auto ok = result.SetFromJson(json);

    ASSERT_THAT(ok, Eq(true));

    ASSERT_THAT(result.id, Eq(finfo.id));
    ASSERT_THAT(result.name, Eq(finfo.name));
    ASSERT_THAT(result.size, Eq(finfo.size));
    ASSERT_THAT(result.timestamp, Eq(finfo.timestamp));
    ASSERT_THAT(result.buf_id, Eq(finfo.buf_id));
    ASSERT_THAT(result.source, Eq(finfo.source));
    ASSERT_THAT(result.metadata, Eq(finfo.metadata));

}


TEST(FileInFo, CorrectConvertFromJsonEmptyMeta) {
    auto finfo = PrepareFileInfo();
    finfo.metadata = "";
    std::string json = finfo.Json();

    FileInfo result;
    auto ok = result.SetFromJson(json);

    ASSERT_THAT(ok, Eq(true));
    ASSERT_THAT(result.metadata, Eq("{}"));

}


TEST(FileInFo, EpochNanosecsFromNow) {
    auto ns = asapo::EpochNanosecsFromNow();
    ASSERT_THAT(ns, ::testing::Gt(0));
}


struct TestEpochFromISODate {
    std::string iso;
    uint64_t ns;
};

auto tests = std::vector<TestEpochFromISODate> {
    TestEpochFromISODate{"1970-01-01T00:00:00.0Z", 1}, // 0 reserved for errors
    TestEpochFromISODate{"1970-01-01Z", 1},
    TestEpochFromISODate{"1970-01-01T00:00:00.000000002Z", 2},
    TestEpochFromISODate{"2019-07-25T15:38:11.100010002Z", 1564069091100010002},
//errors
    TestEpochFromISODate{"1970-13-01T00:00:00.000000002", 0},
    TestEpochFromISODate{"1970-12-01T00:00:00.", 0},
    TestEpochFromISODate{"1970-01-01T00:00:00.000000002", 0},
};

TEST(FileInFo, NanosecsEpochFromISODate) {
    for (auto test : tests) {
        auto res = asapo::NanosecsEpochFromISODate(test.iso);
        ASSERT_THAT(res, Eq(test.ns));
    }
}

auto tests2 = std::vector<TestEpochFromISODate> {
    TestEpochFromISODate{"1970-01-01T00:00:00Z", 0},
    TestEpochFromISODate{"1970-01-01T00:00:00.000000002Z", 2},
    TestEpochFromISODate{"2019-07-25T15:38:11.100010002Z", 1564069091100010002},
};

TEST(FileInFo, ISODateFromNanosecsEpoch) {
    for (auto test : tests2) {
        auto res = asapo::IsoDateFromEpochNanosecs(test.ns);
        ASSERT_THAT(res, Eq(test.iso));
    }
}


StreamInfo PrepareStreamInfo() {
    StreamInfo sinfo;
    sinfo.last_id = 123;
    return sinfo;
}


TEST(StreamInfo, ConvertFromJson) {
    StreamInfo result;

    std::string json = R"({"lastId":123})";
    auto ok = result.SetFromJson(json);

    ASSERT_THAT(ok, Eq(true));
    ASSERT_THAT(result.last_id, Eq(123));
}


TEST(StreamInfo, ConvertFromJsonErr) {
    StreamInfo result;

    std::string json = R"({"lastId":123)";
    auto ok = result.SetFromJson(json);

    ASSERT_THAT(ok, Eq(false));
    ASSERT_THAT(result.last_id, Eq(0));
}

TEST(StreamInfo, ConvertToJson) {
    auto sinfo = PrepareStreamInfo();

    std::string expected_json = R"({"lastId":123})";
    auto json = sinfo.Json();

    ASSERT_THAT(expected_json, Eq(json));
}

TEST(SourceCredentials, ConvertToString) {
    auto sc = SourceCredentials{SourceType::kRaw,"beamtime","beamline","stream","token"};
    std::string expected1= "raw%beamtime%beamline%stream%token";
    std::string expected2= "processed%beamtime%beamline%stream%token";

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


}
