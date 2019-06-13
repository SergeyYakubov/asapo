#include "common/data_structs.h"
#include "preprocessor/definitions.h"
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include <chrono>


using asapo::FileInfo;

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
    finfo.modify_date = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(1));
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
    printf("%s\n", json.c_str());
    if (asapo::kPathSeparator == '/') {
        ASSERT_THAT(json, Eq(
                        R"({"_id":1,"size":100,"name":"folder/test","lastchange":1000000,"source":"host:1234","buf_id":-1,"meta":{"bla":10}})"));
    } else {
        ASSERT_THAT(json, Eq(
                        R"({"_id":1,"size":100,"name":"folder\\test","lastchange":1000000,"source":"host:1234","buf_id":-1,"meta":{"bla":10}})"));
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
    ASSERT_THAT(result.modify_date, Eq(finfo.modify_date));
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



}
