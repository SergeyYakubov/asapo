#include "receiver_statistics.h"

namespace asapo {

using std::chrono::system_clock;

ReceiverStatistics::ReceiverStatistics(unsigned int write_interval) : Statistics(write_interval) {
    for (size_t i = 0; i < kNStatisticEntities; i++) {
        time_counters_[i] = std::chrono::nanoseconds{0};
    }
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

void ReceiverStatistics::ApplyTimeFrom(const SharedInstancedStatistics& stats) {
    for (size_t i = 0; i < kNStatisticEntities; i++) {
        time_counters_[i] += stats->GetElapsed((StatisticEntity) i);
    }
}

}
