#include "common/data_structs.h"

#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include <chrono>


using hidra2::FileInfo;

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

FileInfo PrepareFileInfo() {
    FileInfo finfo;
    finfo.size = 100;
    finfo.id = 1;
    finfo.relative_path = "relative_path";
    finfo.base_name = "base_name";
    finfo.modify_date = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(1));
    return finfo;
}

TEST(FileInFo, CorrectConvertToJson) {
    auto finfo = PrepareFileInfo();
    std::string json = finfo.Json();
    ASSERT_THAT(json, Eq(
                    R"({"_id":1,"size":100,"base_name":"base_name","lastchange":1000000,"relative_path":"relative_path"})"));
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

TEST(FileInFo, CorrectConvertFromJson) {
    auto finfo = PrepareFileInfo();
    std::string json = finfo.Json();

    FileInfo result;
    auto ok = result.SetFromJson(json);
    ASSERT_THAT(result.id, Eq(finfo.id));
    ASSERT_THAT(result.base_name, Eq(finfo.base_name));
    ASSERT_THAT(result.size, Eq(finfo.size));
    ASSERT_THAT(result.modify_date, Eq(finfo.modify_date));
    ASSERT_THAT(result.relative_path, Eq(finfo.relative_path));
    ASSERT_THAT(ok, Eq(true));

}



}