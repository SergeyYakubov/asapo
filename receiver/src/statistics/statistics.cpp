#include "statistics.h"
#include "statistics_sender_influx_db.h"
#include "statistics_sender_fluentd.h"
#include "../receiver_config.h"
#include <algorithm>

using std::chrono::system_clock;

namespace asapo {

void Statistics::SendIfNeeded(bool send_always) noexcept {
    if (!GetReceiverConfig()->monitor_performance) {
        return;
    }
    if (send_always || GetTotalElapsedMs() > write_interval_) {
        std::lock_guard<std::mutex> lock{mutex_};
        Send();
    }
}

void Statistics::Send() noexcept {
    for (auto& sender : statistics_sender_list__) {
        sender->SendStatistics(PrepareStatisticsToSend());
    }
    ResetStatistics();
}


StatisticsToSend Statistics::PrepareStatisticsToSend() const noexcept {
    StatisticsToSend stat;
    stat.n_requests = nrequests_;
    stat.data_volume = volume_counter_;
    stat.elapsed_ms = std::max(uint64_t{1}, GetTotalElapsedMs());
    stat.tags = tags_;
    return stat;
}

uint64_t Statistics::GetTotalElapsedMs() const noexcept {
    return std::chrono::duration_cast<std::chrono::milliseconds>
           ( system_clock::now() - last_timepoint_).count();
}


void Statistics::SetWriteInterval(uint64_t interval_ms) {
    write_interval_ = (size_t) interval_ms;
}

void Statistics::ResetStatistics() noexcept {
    last_timepoint_ = system_clock::now();
    nrequests_ = 0;
    volume_counter_ = 0;
}

void Statistics::IncreaseRequestCounter() noexcept {
    std::lock_guard<std::mutex> lock{mutex_};
    nrequests_++;
}

Statistics::Statistics(unsigned int write_frequency) :
    write_interval_{write_frequency} {
    statistics_sender_list__.emplace_back(std::unique_ptr<StatisticsSender> {new StatisticsSenderInfluxDb});
//    statistics_sender_list__.emplace_back(new StatisticsSenderFluentd);
    ResetStatistics();
}


void Statistics::IncreaseRequestDataVolume(uint64_t transferred_data_volume) noexcept {
    std::lock_guard<std::mutex> lock{mutex_};
    volume_counter_ += transferred_data_volume;
}

void Statistics::AddTag(const std::string& name, const std::string& value) noexcept {
    tags_.push_back(std::make_pair(name, value));
}

}
