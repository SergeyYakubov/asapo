#include "statistics.h"
#include "statistics_sender_influx_db.h"
#include <algorithm>

using std::chrono::high_resolution_clock;

namespace asapo {

void Statistics::SendIfNeeded() noexcept {
    if (GetTotalElapsedMs() > write_interval_) {
        Send();
    }
}

void Statistics::Send() noexcept {
    statistics_sender__->SendStatistics(PrepareStatisticsToSend());
    ResetStatistics();
}


StatisticsToSend Statistics::PrepareStatisticsToSend() const noexcept {
    StatisticsToSend stat;
    stat.n_requests = nrequests_;
    stat.data_volume = volume_counter_;
    stat.elapsed_ms = std::max(uint64_t{1}, GetTotalElapsedMs());
    stat.tags = tag_;
    for (auto i = 0; i < kNStatisticEntities; i++) {
        stat.entity_shares[i] =  double(GetElapsedMs(StatisticEntity(i))) / stat.elapsed_ms;
    }
    return stat;
}

uint64_t Statistics::GetTotalElapsedMs() const noexcept {
    return std::chrono::duration_cast<std::chrono::milliseconds>
           ( high_resolution_clock::now() - last_timepoint_).count();
}

uint64_t Statistics::GetElapsedMs(StatisticEntity entity) const noexcept {
    return std::chrono::duration_cast<std::chrono::milliseconds>(time_counters_[entity]).count();
}

void Statistics::SetWriteInterval(uint64_t interval_ms) {
    write_interval_ = interval_ms;
}

void Statistics::ResetStatistics() noexcept {
    last_timepoint_ = high_resolution_clock::now();
    nrequests_ = 0;
    for (int i = 0; i < kNStatisticEntities; i++) {
        time_counters_[i] = std::chrono::nanoseconds{0};
    }
    volume_counter_ = 0;
}

void Statistics::IncreaseRequestCounter() noexcept {
    nrequests_++;
}

Statistics::Statistics(unsigned int write_frequency) : statistics_sender__{new StatisticsSenderInfluxDb},
write_interval_{write_frequency} {
    ResetStatistics();
}

void Statistics::StartTimer(const StatisticEntity& entity) noexcept {
    current_statistic_entity_ = entity;
    current_timer_last_timepoint_ = high_resolution_clock::now();
}

void Statistics::IncreaseRequestDataVolume(uint64_t transferred_data_volume) noexcept {
    volume_counter_ += transferred_data_volume;
}


void Statistics::StopTimer() noexcept {
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>
                   (high_resolution_clock::now() - current_timer_last_timepoint_);
    time_counters_[current_statistic_entity_] += elapsed;
}

void Statistics::AddTag(const std::string &name, const std::string &value) noexcept {
    if (!tag_.empty()) {
        tag_ += ",";
    }
    tag_ += name + "=" + value;
}

}