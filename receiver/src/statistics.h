#ifndef ASAPO_STATISTICS_H
#define ASAPO_STATISTICS_H

#include <chrono>
#include <memory>
#include <string>


#include "statistics_sender.h"

namespace asapo {

static const auto kNStatisticEntities = 3;
enum StatisticEntity : int {
    kDatabase = 0,
    kDisk,
    kNetwork,
};

struct StatisticsToSend {
    double entity_shares[kNStatisticEntities];
    uint64_t elapsed_ms;
    uint64_t data_volume;
    uint64_t n_requests;
    std::string tags;
};

#ifdef UNIT_TESTS
#define VIRTUAL virtual
#endif

class Statistics {
  public:
    VIRTUAL void SendIfNeeded() noexcept;
    VIRTUAL void Send() noexcept;
    explicit Statistics(unsigned int write_interval = kDefaultStatisticWriteIntervalMs);
    VIRTUAL void IncreaseRequestCounter() noexcept;
    VIRTUAL void StartTimer(const StatisticEntity& entity) noexcept;
    VIRTUAL void IncreaseRequestDataVolume(uint64_t transferred_data_volume) noexcept;
    VIRTUAL void StopTimer() noexcept;
    VIRTUAL void AddTag(const std::string& name,const std::string& value) noexcept;


  void SetWriteInterval(uint64_t interval_ms);
    std::unique_ptr<StatisticsSender> statistics_sender__;
  private:
    uint64_t GetElapsedMs(StatisticEntity entity) const noexcept;
    void ResetStatistics() noexcept;
    uint64_t GetTotalElapsedMs() const noexcept;
    StatisticsToSend PrepareStatisticsToSend() const noexcept;
    static const unsigned int kDefaultStatisticWriteIntervalMs = 10000;
    uint64_t nrequests_;
    std::chrono::high_resolution_clock::time_point last_timepoint_;
    std::chrono::high_resolution_clock::time_point current_timer_last_timepoint_;
    StatisticEntity current_statistic_entity_ = StatisticEntity::kDatabase;
    std::chrono::nanoseconds time_counters_[kNStatisticEntities];
    uint64_t volume_counter_;
    unsigned int write_interval_;
    std::string tag_;

};

}

#endif //ASAPO_STATISTICS_H
