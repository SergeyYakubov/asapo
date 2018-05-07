#ifndef HIDRA2_STATISTICS_SENDER_INFLUX_DB_H
#define HIDRA2_STATISTICS_SENDER_INFLUX_DB_H

#include "http_client/http_client.h"
#include "statistics_sender.h"
#include "logger/logger.h"

namespace hidra2 {

class StatisticsSenderInfluxDb : public StatisticsSender {
  public:
    StatisticsSenderInfluxDb();
    virtual void SendStatistics(const StatisticsToSend& statistic) const noexcept override;
    std::unique_ptr<HttpClient> httpclient__;
    const AbstractLogger* log__;
  private:
    std::string StatisticsToString(const StatisticsToSend& statistic) const noexcept;

};

}

#endif //HIDRA2_STATISTICS_SENDER_INFLUX_DB_H
