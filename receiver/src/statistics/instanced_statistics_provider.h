#ifndef ASAPO_INSTANCED_STATISTICS_PROVIDER_H
#define ASAPO_INSTANCED_STATISTICS_PROVIDER_H

#include <chrono>
#include <asapo/preprocessor/definitions.h>
#include <memory>
#include "receiver_statistics_entry_type.h"

namespace asapo {

class RequestStatistics {
private:
    std::chrono::system_clock::time_point current_timer_last_timepoint_;
    StatisticEntity current_statistic_entity_ = StatisticEntity::kDatabase;
    std::chrono::nanoseconds time_counters_[kNStatisticEntities] {std::chrono::nanoseconds(0)};

    uint64_t incomingBytes_ = 0;
    uint64_t outgoingBytes_ = 0;
public:
    ASAPO_VIRTUAL void StartTimer(StatisticEntity entity);
    ASAPO_VIRTUAL void StopTimer();

    ASAPO_VIRTUAL void AddIncomingBytes(uint64_t incomingByteCount);
    ASAPO_VIRTUAL void AddOutgoingBytes(uint64_t outgoingByteCount);

    ASAPO_VIRTUAL uint64_t GetIncomingBytes() const;
    ASAPO_VIRTUAL uint64_t GetOutgoingBytes() const;

    ASAPO_VIRTUAL std::chrono::nanoseconds GetElapsed(StatisticEntity entity) const;
    ASAPO_VIRTUAL uint64_t GetElapsedMicrosecondsCount(StatisticEntity entity) const;

    ASAPO_VIRTUAL ~RequestStatistics() = default;

};

using RequestStatisticsPtr = std::unique_ptr<RequestStatistics>;

}

#endif //ASAPO_INSTANCED_STATISTICS_PROVIDER_H
