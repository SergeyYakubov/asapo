#ifndef ASAPO_RECEIVER_STATISTICS_H
#define ASAPO_RECEIVER_STATISTICS_H

#include "statistics.h"
#include "receiver_statistics_entry_type.h"
#include "instanced_statistics_provider.h"

namespace asapo {

static const std::vector<std::string> kStatisticEntityNames = {
        "db_share",
        "disk_share",
        "network_incoming_share",
        "network_outgoing_share",
        "monitoring"
};

class ReceiverStatistics : public Statistics {
public:
    ReceiverStatistics(unsigned int write_interval = kDefaultStatisticWriteIntervalMs);
    void ApplyTimeFrom(const SharedInstancedStatistics& stats);
private:
    StatisticsToSend PrepareStatisticsToSend() const noexcept override;
    void ResetStatistics() noexcept override;
    uint64_t GetElapsedMs(StatisticEntity entity) const noexcept;
    std::chrono::nanoseconds time_counters_[kNStatisticEntities];
};

}

#endif //ASAPO_RECEIVER_STATISTICS_H
