#ifndef ASAPO_INSTANCED_STATISTICS_PROVIDER_H
#define ASAPO_INSTANCED_STATISTICS_PROVIDER_H

#include <chrono>
#include <asapo/preprocessor/definitions.h>
#include <memory>
#include "receiver_statistics_entry_type.h"

namespace asapo {

class InstancedStatistics {
private:
    std::chrono::system_clock::time_point current_timer_last_timepoint_;
    StatisticEntity current_statistic_entity_ = StatisticEntity::kDatabase;
    std::chrono::nanoseconds time_counters_[kNStatisticEntities] {std::chrono::nanoseconds(0)};

    uint64_t incomingBytes_ = 0;
    uint64_t outgoingBytes_ = 0;
public:
    VIRTUAL void StartTimer(StatisticEntity entity);
    VIRTUAL void StopTimer();

    VIRTUAL void AddIncomingBytes(uint64_t incomingByteCount);
    VIRTUAL void AddOutgoingBytes(uint64_t outgoingByteCount);

    VIRTUAL uint64_t GetIncomingBytes() const;
    VIRTUAL uint64_t GetOutgoingBytes() const;

    VIRTUAL std::chrono::nanoseconds GetElapsed(StatisticEntity entity) const;
    VIRTUAL uint64_t GetElapsedMicrosecondsCount(StatisticEntity entity) const;
};

using SharedInstancedStatistics = std::shared_ptr<asapo::InstancedStatistics>;

}

#endif //ASAPO_INSTANCED_STATISTICS_PROVIDER_H
