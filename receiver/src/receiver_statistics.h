#ifndef ASAPO_RECEIVER_STATISTICS_H
#define ASAPO_RECEIVER_STATISTICS_H

#include "statistics.h"

namespace asapo {

static const auto kNStatisticEntities = 3;
enum StatisticEntity : int {
    kDatabase = 0,
    kDisk,
    kNetwork,
    kAuthorizer
};

static const std::vector<std::string> kStatisticEntityNames = {"db_share", "disk_share", "network_share"};

class ReceiverStatistics : public Statistics {
  public:
    ReceiverStatistics(unsigned int write_interval = kDefaultStatisticWriteIntervalMs);
    VIRTUAL void StartTimer(const StatisticEntity& entity) noexcept;
    VIRTUAL void StopTimer() noexcept;
  private:
    StatisticsToSend PrepareStatisticsToSend() const noexcept override;
    void ResetStatistics() noexcept override;
    uint64_t GetElapsedMs(StatisticEntity entity) const noexcept;
    std::chrono::high_resolution_clock::time_point current_timer_last_timepoint_;
    StatisticEntity current_statistic_entity_ = StatisticEntity::kDatabase;
    std::chrono::nanoseconds time_counters_[kNStatisticEntities];
};

}

#endif //ASAPO_RECEIVER_STATISTICS_H
