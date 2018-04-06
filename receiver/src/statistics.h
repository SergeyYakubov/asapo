#ifndef HIDRA2_STATISTICS_H
#define HIDRA2_STATISTICS_H

#include <chrono>

namespace hidra2 {

enum StatisticEntity : int {
    kDatabase = 0,
    kDisk,
    kNetwork,
};

class Statistics {
  public:
    virtual void SendIfNeeded() const;
    Statistics();
    double GetRate() const noexcept;
    double GetBandwidth() const noexcept;
    void IncreaseRequestCounter() noexcept;
    void StartTimer(const StatisticEntity& entity) noexcept;
    void IncreaseRequestDataVolume(uint64_t transferred_data_volume) noexcept;
    void StopTimer() noexcept;
  private:
    void ResetStatistics() noexcept;
    static const auto kNStatisticEntities = 3;
    uint64_t nrequests_;
    std::chrono::high_resolution_clock::time_point last_timepoint_;
    std::chrono::high_resolution_clock::time_point current_timer_last_timepoint_;
    StatisticEntity current_statistic_entity_ = StatisticEntity::kDatabase;
    std::chrono::nanoseconds time_counters_[kNStatisticEntities];
    uint64_t volume_counter_;

};

}

#endif //HIDRA2_STATISTICS_H
