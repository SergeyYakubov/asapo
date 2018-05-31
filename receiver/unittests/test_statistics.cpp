#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>

#include "../src/statistics.h"
#include "../src/statistics_sender.h"
#include "../src/statistics_sender_influx_db.h"

using ::testing::Test;
using ::testing::Gt;
using ::testing::Ge;
using ::testing::Le;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Ref;
using ::testing::_;

using asapo::Statistics;
using asapo::StatisticEntity;
using asapo::StatisticsSender;
using asapo::StatisticsSenderInfluxDb;
using asapo::StatisticsToSend;



namespace {


TEST(StatisticTestsConstructor, Constructor) {
    Statistics statistics;
    ASSERT_THAT(dynamic_cast<asapo::StatisticsSenderInfluxDb*>(statistics.statistics_sender__.get()), Ne(nullptr));
}


class MockStatisticsSender: public StatisticsSender {
  public:
    void SendStatistics(const StatisticsToSend& statistics) const noexcept override {
        SendStatistics_t(statistics);
    }
    MOCK_CONST_METHOD1(SendStatistics_t, void (const StatisticsToSend&));
};

class StatisticTests : public Test {
  public:
    Statistics statistics{0};
    void TestTimer(const StatisticEntity& entity);
    MockStatisticsSender mock_statistics_sender;
    void SetUp() override {
        statistics.statistics_sender__.reset(&mock_statistics_sender);
    }
    void TearDown() override {
        statistics.statistics_sender__.release();
    }
    StatisticsToSend ExtractStat();
};


ACTION_P(SaveArg1ToSendStat, value) {
    auto resp =  static_cast<const StatisticsToSend&>(arg0);
    value->n_requests = resp.n_requests;
    value->data_volume = resp.data_volume;
    value->elapsed_ms = resp.elapsed_ms;
    value->tags = resp.tags;
    for (int i = 0; i < asapo::kNStatisticEntities; i++) {
        value->entity_shares[i] = resp.entity_shares[i];
    }

}


StatisticsToSend StatisticTests::ExtractStat() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    StatisticsToSend stat;
    stat.elapsed_ms = 0;
    stat.n_requests = 0;
    stat.data_volume = 0;
    for (int i = 0; i < asapo::kNStatisticEntities; i++) {
        stat.entity_shares[i] = 0.0;
    }

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

    ASSERT_THAT(stat.tags, Eq("name=value"));
}

TEST_F(StatisticTests, AddTagTwice) {
    statistics.AddTag("name1", "value1");
    statistics.AddTag("name2", "value2");

    auto stat = ExtractStat();

    ASSERT_THAT(stat.tags, Eq("name1=value1,name2=value2"));
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

void StatisticTests::TestTimer(const StatisticEntity& entity) {
    statistics.StartTimer(entity);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    statistics.StopTimer();

    auto stat = ExtractStat();

    ASSERT_THAT(stat.entity_shares[entity], Ge(0.4));

}

TEST_F(StatisticTests, TimerForDatabase) {
    TestTimer(StatisticEntity::kDatabase);
}

TEST_F(StatisticTests, TimerForNetwork) {
    TestTimer(StatisticEntity::kNetwork);
}

TEST_F(StatisticTests, TimerForDisk) {
    TestTimer(StatisticEntity::kDisk);
}

TEST_F(StatisticTests, TimerForAll) {
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

    ASSERT_THAT(stat.entity_shares[StatisticEntity::kDatabase], Ge(0.15));
    ASSERT_THAT(stat.entity_shares[StatisticEntity::kDatabase], Le(0.25));

    ASSERT_THAT(stat.entity_shares[StatisticEntity::kNetwork], Ge(0.25));
    ASSERT_THAT(stat.entity_shares[StatisticEntity::kNetwork], Le(0.35));

    ASSERT_THAT(stat.entity_shares[StatisticEntity::kDisk], Ge(0.35));
    ASSERT_THAT(stat.entity_shares[StatisticEntity::kDisk], Le(0.45));
}


TEST_F(StatisticTests, SendStaticsDoesCallsSender) {
    statistics.SetWriteInterval(1000);

    EXPECT_CALL(mock_statistics_sender, SendStatistics_t(_)).Times(0);

    statistics.SendIfNeeded();
}


TEST_F(StatisticTests, StatisticsSend) {
    statistics.IncreaseRequestCounter();

    StatisticsToSend stat;
    stat.elapsed_ms = 0;
    stat.n_requests = 0;
    stat.data_volume = 0;
    for (int i = 0; i < asapo::kNStatisticEntities; i++) {
        stat.entity_shares[i] = 0.0;
    }

    EXPECT_CALL(mock_statistics_sender, SendStatistics_t(_)).
    WillOnce(SaveArg1ToSendStat(&stat));

    statistics.Send();
    std::cout << stat.elapsed_ms << std::endl;

    ASSERT_THAT(stat.elapsed_ms, Ge(1));
}


}
