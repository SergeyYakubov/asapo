#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>

#include "../../src/statistics/receiver_statistics.h"
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

    statistics.SendIfNeeded(false);
    return stat;
}


void ReceiverStatisticTests::TestTimer(const StatisticEntity& entity) {
    asapo::SharedInstancedStatistics instancedStatistics{new asapo::InstancedStatistics};

    instancedStatistics->StartTimer(entity);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    instancedStatistics->StopTimer();

    statistics.ApplyTimeFrom(instancedStatistics);

    auto stat = ExtractStat();

    ASSERT_THAT(stat.extra_entities.size(), Gt(static_cast<size_t>(entity)));
    ASSERT_THAT(stat.extra_entities[static_cast<size_t>(entity)].second, Ge(0.3));
    ASSERT_THAT(stat.extra_entities[static_cast<size_t>(entity)].second, Le(1.0));
}

TEST_F(ReceiverStatisticTests, TimerForDatabase) {
    TestTimer(StatisticEntity::kDatabase);
}

TEST_F(ReceiverStatisticTests, TimerForNetworkIncoming) {
    TestTimer(StatisticEntity::kNetworkIncoming);
}

TEST_F(ReceiverStatisticTests, TimerForNetworkOutgoing) {
    TestTimer(StatisticEntity::kNetworkOutgoing);
}

TEST_F(ReceiverStatisticTests, TimerForDisk) {
    TestTimer(StatisticEntity::kDisk);
}

TEST_F(ReceiverStatisticTests, TimerForMonitoring) {
    TestTimer(StatisticEntity::kMonitoring);
}

TEST_F(ReceiverStatisticTests, ByteCounter) {
    asapo::SharedInstancedStatistics instancedStatistics{new asapo::InstancedStatistics};

    instancedStatistics->AddIncomingBytes(53);
    instancedStatistics->AddIncomingBytes(23);

    instancedStatistics->AddOutgoingBytes(5);
    instancedStatistics->AddOutgoingBytes(7);

    ASSERT_THAT(instancedStatistics->GetIncomingBytes(), Eq(76));
    ASSERT_THAT(instancedStatistics->GetOutgoingBytes(), Eq(12));
}

TEST_F(ReceiverStatisticTests, TimerForAll) {
    asapo::SharedInstancedStatistics instancedStatistics{new asapo::InstancedStatistics};

    // kDatabase
    instancedStatistics->StartTimer(StatisticEntity::kDatabase);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    instancedStatistics->StopTimer();

    // kNetworkIncoming
    instancedStatistics->StartTimer(StatisticEntity::kNetworkIncoming);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    instancedStatistics->StopTimer();

    // kNetworkOutgoing
    instancedStatistics->StartTimer(StatisticEntity::kNetworkOutgoing);
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    instancedStatistics->StopTimer();

    // kDisk
    instancedStatistics->StartTimer(StatisticEntity::kDisk);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    instancedStatistics->StopTimer();

    // kMonitoring
    instancedStatistics->StartTimer(StatisticEntity::kMonitoring);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    instancedStatistics->StopTimer();

    statistics.ApplyTimeFrom(instancedStatistics);

    auto stat = ExtractStat();

    ASSERT_THAT(stat.extra_entities.size(), Gt(static_cast<size_t>(StatisticEntity::kDatabase)));
    ASSERT_THAT(stat.extra_entities[StatisticEntity::kDatabase].second, Gt(0));

    ASSERT_THAT(stat.extra_entities.size(), Gt(static_cast<size_t>(StatisticEntity::kNetworkIncoming)));
    ASSERT_THAT(stat.extra_entities[StatisticEntity::kNetworkIncoming].second, Gt(0));

    ASSERT_THAT(stat.extra_entities.size(), Gt(static_cast<size_t>(StatisticEntity::kNetworkOutgoing)));
    ASSERT_THAT(stat.extra_entities[StatisticEntity::kNetworkOutgoing].second, Gt(0));

    ASSERT_THAT(stat.extra_entities.size(), Gt(static_cast<size_t>(StatisticEntity::kDisk)));
    ASSERT_THAT(stat.extra_entities[StatisticEntity::kDisk].second, Gt(0));

    ASSERT_THAT(stat.extra_entities.size(), Gt(static_cast<size_t>(StatisticEntity::kMonitoring)));
    ASSERT_THAT(stat.extra_entities[StatisticEntity::kMonitoring].second, Gt(0));
}

}
