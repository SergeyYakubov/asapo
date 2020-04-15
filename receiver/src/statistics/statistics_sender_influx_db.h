#ifndef ASAPO_STATISTICS_SENDER_INFLUX_DB_H
#define ASAPO_STATISTICS_SENDER_INFLUX_DB_H

#include "http_client/http_client.h"
#include "statistics_sender.h"
#include "logger/logger.h"

namespace asapo {

class StatisticsSenderInfluxDb : public StatisticsSender {
  public:
    StatisticsSenderInfluxDb();
    virtual void SendStatistics(const StatisticsToSend& statistic) const noexcept override;
    std::unique_ptr<HttpClient> httpclient__;
    const AbstractLogger* log__;
    ~StatisticsSenderInfluxDb() {};
  private:
    std::string StatisticsToString(const StatisticsToSend& statistic) const noexcept;

};

}

#endif //ASAPO_STATISTICS_SENDER_INFLUX_DB_H
