#include "statistics.h"

using std::chrono::high_resolution_clock;

namespace hidra2 {

void Statistics::SendIfNeeded() const {

}

double Statistics::GetRate() const noexcept {
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>
                      ( high_resolution_clock::now() - last_timepoint_).count();
    if (elapsed_ms == 0) {
        return 0.0;
    } else {
        return double(nrequests_) / elapsed_ms * 1000.0;
    }
}

double Statistics::GetBandwidth() const noexcept {
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>
                      ( high_resolution_clock::now() - last_timepoint_).count();
    if (elapsed_ms == 0) {
        return 0.0;
    } else {
        return double(volume_counter_) / elapsed_ms * 1000.0 / (1024.0 * 1024.0 * 1024.0);
    }
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

Statistics::Statistics() {
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


}