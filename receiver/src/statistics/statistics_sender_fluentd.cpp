#include "statistics_sender_fluentd.h"
#include "statistics.h"

namespace asapo {

StatisticsSenderFluentd::StatisticsSenderFluentd() : statistics_log__{asapo::CreateDefaultLoggerApi("receiver_stat", "localhost:8400/logs/")} {
    statistics_log__->SetLogLevel(LogLevel::Info);
}

void StatisticsSenderFluentd::SendStatistics(const asapo::StatisticsToSend& statistic) const noexcept {
    statistics_log__->Info(StatisticsToString(statistic));
}

std::string StatisticsSenderFluentd::StatisticsToString(const asapo::StatisticsToSend& statistic) const noexcept {
    LogMessageWithFields msg{"@target_type_key", "stat"};
    for (auto tag : statistic.tags) {
        msg.Append(tag.first, tag.second);
    }

    msg.Append("elapsed_ms", statistic.elapsed_ms);
    msg.Append("data_volume", statistic.data_volume);
    msg.Append("n_requests", statistic.n_requests);
    for (const auto& entity : statistic.extra_entities) {
        msg.Append(entity.first, entity.second, 4);
    }
    return msg.LogString();
}

}
