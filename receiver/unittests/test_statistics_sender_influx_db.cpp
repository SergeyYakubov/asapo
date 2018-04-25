#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>

#include "../src/statistics_sender_influx_db.h"
#include "../src/statistics_sender.h"
#include "../../common/cpp/src/http_client/curl_http_client.h"
#include "unittests/MockHttpClient.h"
#include "../src/statistics.h"

#include "../src/receiver_config.h"
#include "mock_receiver_config.h"


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
using hidra2::MockHttpClient;
using hidra2::StatisticsToSend;
using hidra2::ReceiverConfig;
using hidra2::SetReceiverConfig;

namespace {

TEST(SenderInfluxDb, Constructor) {
    StatisticsSenderInfluxDb sender;
    ASSERT_THAT(dynamic_cast<hidra2::CurlHttpClient*>(sender.httpclient__.get()), Ne(nullptr));
}


class SenderInfluxDbTests : public Test {
  public:
    StatisticsSenderInfluxDb sender;
    MockHttpClient mock_http_client;
    void SetUp() override {
        sender.httpclient__.reset(&mock_http_client);
    }
    void TearDown() override {
        sender.httpclient__.release();
    }
};


TEST_F(SenderInfluxDbTests, SendStatisticsCallsPost) {
    StatisticsToSend statistics;
    statistics.n_requests = 4;
    statistics.entity_shares[hidra2::StatisticEntity::kDisk] = 0.6;
    statistics.entity_shares[hidra2::StatisticEntity::kNetwork] = 0.3;
    statistics.entity_shares[hidra2::StatisticEntity::kDatabase] = 0.1;
    statistics.elapsed_ms = 100;
    statistics.data_volume = 1000;

    ReceiverConfig config;
    config.monitor_db_uri = "test_uri";
    config.monitor_db_name = "test_name";
    SetReceiverConfig(config);

    std::string expect_string = "statistics,receiver=1,connection=1 elapsed_ms=100,data_volume=1000,"
                                "n_requests=4,db_share=0.1000,network_share=0.3000,disk_share=0.6000";
    EXPECT_CALL(mock_http_client, Post_t("test_uri/write?db=test_name", expect_string, _, _)).
    WillOnce(
        DoAll(SetArgPointee<3>(new hidra2::IOError("Test Read Error", hidra2::IOErrorType::kReadError)),
              Return("")
             ));

    sender.SendStatistics(statistics);
}


}
