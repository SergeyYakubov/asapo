#include "instanced_statistics_provider.h"

using namespace asapo;
using namespace std::chrono;

void InstancedStatistics::StartTimer(StatisticEntity entity) {
    current_statistic_entity_ = entity;
    current_timer_last_timepoint_ = system_clock::now();
}

void InstancedStatistics::StopTimer() {
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(system_clock::now() - current_timer_last_timepoint_);
    time_counters_[current_statistic_entity_] += elapsed;
}

std::chrono::nanoseconds InstancedStatistics::GetElapsed(StatisticEntity entity) const {
    return time_counters_[entity];
}

uint64_t InstancedStatistics::GetElapsedMicrosecondsCount(StatisticEntity entity) const {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(time_counters_[entity]).count());
}

void InstancedStatistics::AddIncomingBytes(uint64_t incomingByteCount) {
    incomingBytes_ += incomingByteCount;
}

void InstancedStatistics::AddOutgoingBytes(uint64_t outgoingByteCount) {
    outgoingBytes_ += outgoingByteCount;
}

uint64_t InstancedStatistics::GetIncomingBytes() const {
    return incomingBytes_;
}

uint64_t InstancedStatistics::GetOutgoingBytes() const {
    return outgoingBytes_;
}
