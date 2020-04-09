#ifndef ASAPO_STATISTICS_SENDER_H
#define ASAPO_STATISTICS_SENDER_H

#include <cstdint>

namespace asapo {

struct StatisticsToSend;

class StatisticsSender {
  public:
    virtual void SendStatistics(const StatisticsToSend& statistic) const noexcept = 0;
    virtual ~StatisticsSender() = default;
};

}

#endif //ASAPO_STATISTICS_SENDER_H
