#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>

#include "../src/statistics_sender_influx_db.h"
#include "../src/statistics_sender.h"
#include "http_client/curl_http_client.h"

using ::testing::Test;
using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::InSequence;
using ::testing::SetArgPointee;

using hidra2::StatisticsSenderInfluxDb;


namespace {

TEST(SenderInfluxDb, Constructor) {
    StatisticsSenderInfluxDb sender;
    ASSERT_THAT(dynamic_cast<hidra2::CurlHttpClient*>(sender.httpclient__.get()), Ne(nullptr));
}



}
