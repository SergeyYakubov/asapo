#ifndef ASAPO_STATISTICS_H
#define ASAPO_STATISTICS_H

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <mutex>


#include "statistics_sender.h"
#include "asapo/preprocessor/definitions.h"

namespace asapo {

using ExtraEntity = std::pair<std::string, double>;

struct StatisticsToSend {
    std::vector<ExtraEntity> extra_entities;
    uint64_t elapsed_ms;
    uint64_t data_volume;
    uint64_t n_requests;
    std::vector<std::pair<std::string, std::string>> tags;
};

class Statistics {
  public:
    VIRTUAL void SendIfNeeded(bool send_always = false) noexcept;
    explicit Statistics(unsigned int write_interval = kDefaultStatisticWriteIntervalMs);
    VIRTUAL void IncreaseRequestCounter() noexcept;
    VIRTUAL void IncreaseRequestDataVolume(uint64_t transferred_data_volume) noexcept;
    VIRTUAL void AddTag(const std::string& name, const std::string& value) noexcept;
    void SetWriteInterval(uint64_t interval_ms);
    std::vector<std::unique_ptr<StatisticsSender>> statistics_sender_list__;
    virtual ~Statistics() = default;
  protected:
    static const unsigned int kDefaultStatisticWriteIntervalMs = 10000;
    virtual StatisticsToSend PrepareStatisticsToSend() const noexcept;
    virtual void ResetStatistics() noexcept;
  private:
    void Send() noexcept;
    uint64_t GetTotalElapsedMs() const noexcept;
    uint64_t nrequests_{0};
    std::chrono::system_clock::time_point last_timepoint_{std::chrono::system_clock::now()};
    uint64_t volume_counter_{0};
    unsigned int write_interval_;
    std::vector<std::pair<std::string, std::string>> tags_;
    std::mutex mutex_;

};

}

#endif //ASAPO_STATISTICS_H
