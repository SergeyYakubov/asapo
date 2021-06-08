#include "../../src/database/encoding.h"
#include "asapo/preprocessor/definitions.h"
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include <chrono>

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

TEST(EncodingTests, EncodeDbName) {

    std::string dbname = R"(db_/\."$)";
    std::string dbname_encoded = "db_%2F%5C%2E%22%24";

    auto encoded = asapo::EncodeDbName(dbname);
    auto decoded = asapo::DecodeName(encoded);

    ASSERT_THAT(encoded, Eq(dbname_encoded));
    ASSERT_THAT(decoded, Eq(dbname));
}

TEST(EncodingTests, EncodeColName) {

    std::string colname = R"(col_/\."$)";
    std::string colname_encoded = R"(col_/\."%24)";

    auto encoded = asapo::EncodeColName(colname);

    auto decoded = asapo::DecodeName(encoded);


    ASSERT_THAT(encoded, Eq(colname_encoded));
    ASSERT_THAT(decoded, Eq(colname));
}



}
