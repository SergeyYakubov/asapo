#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"

#include "../src/statistics_sender_influx_db.h"
#include "../src/statistics_sender.h"
#include "../../common/cpp/src/http_client/curl_http_client.h"
#include "unittests/MockHttpClient.h"
#include "../src/statistics.h"

#include "../src/receiver_config.h"
#include "mock_receiver_config.h"
#include "../src/statistics_sender_fluentd.h"

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
using ::testing::HasSubstr;
using ::testing::AllOf;
using ::testing::SaveArgPointee;
using ::testing::InSequence;
using ::testing::SetArgPointee;

using asapo::StatisticsSenderFluentd;
using asapo::MockHttpClient;
using asapo::StatisticsToSend;
using asapo::ReceiverConfig;
using asapo::SetReceiverConfig;

namespace {

TEST(SenderFluentd, Constructor) {
    StatisticsSenderFluentd sender;
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(sender.statistics_log__.get()), Ne(nullptr));
}


class SenderFluentdTests : public Test {
  public:
    StatisticsSenderFluentd sender;
    NiceMock<asapo::MockLogger> mock_logger;
    StatisticsToSend statistics;
    ReceiverConfig config;

    void SetUp() override {
        statistics.n_requests = 4;
        statistics.entity_shares[asapo::StatisticEntity::kDisk] = 0.6;
        statistics.entity_shares[asapo::StatisticEntity::kNetwork] = 0.3;
        statistics.entity_shares[asapo::StatisticEntity::kDatabase] = 0.1;
        statistics.elapsed_ms = 100;
        statistics.data_volume = 1000;
        statistics.tags.push_back(std::make_pair("name1", "value1"));
        statistics.tags.push_back(std::make_pair("name2", "value2"));

        config.monitor_db_uri = "test_uri";
        config.monitor_db_name = "test_name";
        SetReceiverConfig(config, "none");

        sender.statistics_log__.reset(&mock_logger);

    }
    void TearDown() override {
        sender.statistics_log__.release();
    }
};


TEST_F(SenderFluentdTests, SendStatisticsCallsPost) {
    std::string expect_string =
        R"("@target_type_key":"stat","name1":"value1","name2":"value2","elapsed_ms":100,"data_volume":1000)"
        R"(,"n_requests":4,"db_share":0.1000,"network_share":0.3000,"disk_share":0.6000)";
    EXPECT_CALL(mock_logger, Info(HasSubstr(expect_string)));


    sender.SendStatistics(statistics);
}



}
