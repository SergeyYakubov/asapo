
#ifndef ASAPO_STATISTICS_SENDER_FLUENTD_H
#define ASAPO_STATISTICS_SENDER_FLUENTD_H

#include "statistics_sender.h"
#include "logger/logger.h"

namespace asapo {

class StatisticsSenderFluentd : public StatisticsSender {
  public:
    StatisticsSenderFluentd();
    virtual void SendStatistics(const StatisticsToSend& statistic) const noexcept override;
    Logger statistics_log__;
  private:
    std::string StatisticsToString(const StatisticsToSend& statistic) const noexcept;

};

}

#endif //ASAPO_STATISTICS_SENDER_FLUENTD_H
