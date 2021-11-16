#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>

#include "../../src/statistics/receiver_statistics.h"
#include "../../src/statistics/statistics_sender.h"
#include "../../src/statistics/statistics_sender_influx_db.h"
#include "../../src/statistics/statistics_sender_fluentd.h"
#include "../receiver_mocking.h"


using namespace testing;
using namespace asapo;

namespace {


TEST(StatisticTestsConstructor, Constructor) {
    ReceiverStatistics statistics;
    ASSERT_THAT(dynamic_cast<asapo::StatisticsSenderInfluxDb*>(statistics.statistics_sender_list__[0].get()), Ne(nullptr));
//    ASSERT_THAT(dynamic_cast<asapo::StatisticsSenderFluentd*>(statistics.statistics_sender_list__[1].get()), Ne(nullptr));
}

class ReceiverStatisticTests : public Test {
  public:
    ReceiverStatistics statistics{0};
    void TestTimer(const StatisticEntity& entity);
    MockStatisticsSender mock_statistics_sender;
    void SetUp() override {
        statistics.statistics_sender_list__.clear();
        statistics.statistics_sender_list__.emplace_back(&mock_statistics_sender);
    }
    void TearDown() override {
        statistics.statistics_sender_list__[0].release();
    }
    StatisticsToSend ExtractStat();
};


ACTION_P(SaveArg1ToSendStatR, value) {
    auto resp =  static_cast<const StatisticsToSend&>(arg0);
    value->n_requests = resp.n_requests;
    value->data_volume = resp.data_volume;
    value->elapsed_ms = resp.elapsed_ms;
    value->tags = resp.tags;
    for (size_t i = 0; i < asapo::kNStatisticEntities; i++) {
        value->extra_entities.push_back(resp.extra_entities[i]);
    }

}


StatisticsToSend ReceiverStatisticTests::ExtractStat() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    StatisticsToSend stat;
    stat.elapsed_ms = 0;
    stat.n_requests = 0;
    stat.data_volume = 0;

    EXPECT_CALL(mock_statistics_sender, SendStatistics_t(_)).
    WillOnce(SaveArg1ToSendStatR(&stat));

    statistics.SendIfNeeded();
    return stat;
}


void ReceiverStatisticTests::TestTimer(const StatisticEntity& entity) {
    statistics.StartTimer(entity);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    statistics.StopTimer();

    auto stat = ExtractStat();

    ASSERT_THAT(stat.extra_entities[static_cast<size_t>(entity)].second, Gt(0));
    ASSERT_THAT(stat.extra_entities[static_cast<size_t>(entity)].second, Le(1.0));

}

TEST_F(ReceiverStatisticTests, TimerForDatabase) {
    TestTimer(StatisticEntity::kDatabase);
}

TEST_F(ReceiverStatisticTests, TimerForNetwork) {
    TestTimer(StatisticEntity::kNetwork);
}

TEST_F(ReceiverStatisticTests, TimerForDisk) {
    TestTimer(StatisticEntity::kDisk);
}

TEST_F(ReceiverStatisticTests, TimerForAll) {
    statistics.StartTimer(StatisticEntity::kDatabase);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    statistics.StopTimer();
    statistics.StartTimer(StatisticEntity::kNetwork);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    statistics.StopTimer();

    statistics.StartTimer(StatisticEntity::kDisk);
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    statistics.StopTimer();

    auto stat = ExtractStat();

    ASSERT_THAT(stat.extra_entities[StatisticEntity::kDatabase].second, Gt(0));

    ASSERT_THAT(stat.extra_entities[StatisticEntity::kNetwork].second, Gt(0));

    ASSERT_THAT(stat.extra_entities[StatisticEntity::kDisk].second, Gt(0));
}

}
