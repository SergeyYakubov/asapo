#include "receiver_statistics.h"

namespace asapo {

using std::chrono::system_clock;



ReceiverStatistics::ReceiverStatistics(unsigned int write_interval) : Statistics(write_interval) {
    ResetStatistics();
}

StatisticsToSend ReceiverStatistics::PrepareStatisticsToSend() const noexcept {
    StatisticsToSend stat = Statistics::PrepareStatisticsToSend();
    for (size_t i = 0; i < kNStatisticEntities; i++) {
        stat.extra_entities.push_back(ExtraEntity{kStatisticEntityNames[i], double(GetElapsedMs(StatisticEntity(i))) / static_cast<double>(stat.elapsed_ms)});
    }
    return stat;
}

uint64_t ReceiverStatistics::GetElapsedMs(StatisticEntity entity) const noexcept {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(time_counters_[entity]).count());
}

void ReceiverStatistics::ResetStatistics() noexcept {
    Statistics::ResetStatistics();
    for (size_t i = 0; i < kNStatisticEntities; i++) {
        time_counters_[i] = std::chrono::nanoseconds{0};
    }
}

void ReceiverStatistics::StartTimer(const StatisticEntity& entity) noexcept {
    current_statistic_entity_ = entity;
    current_timer_last_timepoint_ = system_clock::now();
}


void ReceiverStatistics::StopTimer() noexcept {
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>
                   (system_clock::now() - current_timer_last_timepoint_);
    time_counters_[current_statistic_entity_] += elapsed;
}


}