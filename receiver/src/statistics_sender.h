#ifndef HIDRA2_STATISTICS_SENDER_H
#define HIDRA2_STATISTICS_SENDER_H

#include <cstdint>

namespace hidra2 {

struct StatisticsToSend;

class StatisticsSender {
 public:
    virtual void SendStatistics(const StatisticsToSend& statistic) const noexcept = 0;
    virtual ~StatisticsSender() = default;
};

}

#endif //HIDRA2_STATISTICS_SENDER_H
