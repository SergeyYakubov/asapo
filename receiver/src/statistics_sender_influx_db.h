#ifndef HIDRA2_STATISTICS_SENDER_INFLUX_DB_H
#define HIDRA2_STATISTICS_SENDER_INFLUX_DB_H

#include "statistics_sender.h"

namespace hidra2 {

class StatisticsSenderInfluxDb : public StatisticsSender{
  virtual void SendStatistics(const StatisticsToSend& statistic) const noexcept override;
};

}

#endif //HIDRA2_STATISTICS_SENDER_INFLUX_DB_H
