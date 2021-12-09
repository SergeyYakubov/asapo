#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>

#include "../../src/statistics/statistics.h"
#include "../../src/statistics/statistics_sender.h"
#include "../../src/statistics/statistics_sender_influx_db.h"
#include "../../src/statistics/statistics_sender_fluentd.h"
#include "../receiver_mocking.h"
#include "../../src/receiver_config.h"
#include "../mock_receiver_config.h"


using namespace testing;
using namespace asapo;


namespace {


TEST(StatisticTestsConstructor, Constructor) {
    Statistics statistics;
    ASSERT_THAT(dynamic_cast<asapo::StatisticsSenderInfluxDb*>(statistics.statistics_sender_list__[0].get()), Ne(nullptr));
//    ASSERT_THAT(dynamic_cast<asapo::StatisticsSenderFluentd*>(statistics.statistics_sender_list__[1].get()), Ne(nullptr));
}


class StatisticTests : public Test {
  public:
    Statistics statistics{0};
    MockStatisticsSender mock_statistics_sender;
    void SetUp() override {
        asapo::ReceiverConfig test_config;
        test_config.monitor_performance = true;
        asapo::SetReceiverConfig(test_config, "none");
        statistics.statistics_sender_list__.clear();
        statistics.statistics_sender_list__.emplace_back(&mock_statistics_sender);
    }
    void TearDown() override {
        statistics.statistics_sender_list__[0].release();
    }
    StatisticsToSend ExtractStat();
};


ACTION_P(SaveArg1ToSendStat, value) {
    auto resp =  static_cast<const StatisticsToSend&>(arg0);
    value->n_requests = resp.n_requests;
    value->data_volume = resp.data_volume;
    value->elapsed_ms = resp.elapsed_ms;
    value->tags = resp.tags;
}


StatisticsToSend StatisticTests::ExtractStat() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    StatisticsToSend stat;
    stat.elapsed_ms = 0;
    stat.n_requests = 0;
    stat.data_volume = 0;

    EXPECT_CALL(mock_statistics_sender, SendStatistics_t(_)).
    WillOnce(SaveArg1ToSendStat(&stat));

    statistics.SendIfNeeded();
    return stat;
}

TEST_F(StatisticTests, IncreaseRequestCounter) {
    statistics.IncreaseRequestCounter();

    auto stat = ExtractStat();

    ASSERT_THAT(stat.n_requests, Eq(1));
}

TEST_F(StatisticTests, AddTag) {
    statistics.AddTag("name", "value");

    auto stat = ExtractStat();

    ASSERT_THAT(stat.tags.size(), Eq(1));
    ASSERT_THAT(stat.tags[0].first, Eq("name"));
    ASSERT_THAT(stat.tags[0].second, Eq("value"));

}

TEST_F(StatisticTests, AddTagTwice) {
    statistics.AddTag("name1", "value1");
    statistics.AddTag("name2", "value2");

    auto stat = ExtractStat();

    ASSERT_THAT(stat.tags.size(), Eq(2));
    ASSERT_THAT(stat.tags[0].first, Eq("name1"));
    ASSERT_THAT(stat.tags[0].second, Eq("value1"));
    ASSERT_THAT(stat.tags[1].first, Eq("name2"));
    ASSERT_THAT(stat.tags[1].second, Eq("value2"));

}




TEST_F(StatisticTests, StatisticsResetAfterSend) {
    statistics.IncreaseRequestCounter();

    ExtractStat();
    auto stat = ExtractStat();

    ASSERT_THAT(stat.n_requests, Eq(0));
}


TEST_F(StatisticTests, ElapsedTime) {

    auto stat = ExtractStat();

    ASSERT_THAT(stat.elapsed_ms, Ge(1));
}



TEST_F(StatisticTests, RequestCounterZeroAtInit) {
    auto stat = ExtractStat();

    ASSERT_THAT(stat.n_requests, Eq(0));
}

TEST_F(StatisticTests, GetDataVolume) {
    statistics.IncreaseRequestDataVolume(100);

    auto stat = ExtractStat();

    ASSERT_THAT(stat.data_volume, Eq(100));
}

TEST_F(StatisticTests, DataVolumeZeroAtInit) {
    auto stat = ExtractStat();

    ASSERT_THAT(stat.data_volume, Eq(0));
}

TEST_F(StatisticTests, SendStaticsDoesCallsSender) {
    statistics.SetWriteInterval(1000);

    EXPECT_CALL(mock_statistics_sender, SendStatistics_t(_)).Times(0);

    statistics.SendIfNeeded();
}


TEST_F(StatisticTests, DoNotSendStatistics) {
    asapo::ReceiverConfig test_config;
    test_config.monitor_performance = false;
    asapo::SetReceiverConfig(test_config, "none");

    EXPECT_CALL(mock_statistics_sender, SendStatistics_t(_)).Times(0);

    statistics.SendIfNeeded(true);
}

TEST_F(StatisticTests, StatisticsSend) {
    statistics.IncreaseRequestCounter();

    StatisticsToSend stat;
    stat.elapsed_ms = 0;
    stat.n_requests = 0;
    stat.data_volume = 0;

    EXPECT_CALL(mock_statistics_sender, SendStatistics_t(_)).
    WillOnce(SaveArg1ToSendStat(&stat));

    statistics.SendIfNeeded(true);
    std::cout << stat.elapsed_ms << std::endl;

    ASSERT_THAT(stat.elapsed_ms, Ge(1));
}


}
