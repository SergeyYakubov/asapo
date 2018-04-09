#include "statistics_sender_influx_db.h"
#include "statistics.h"
#include "http_client/curl_http_client.h"

namespace hidra2 {

void StatisticsSenderInfluxDb::SendStatistics(const StatisticsToSend& statistic) const noexcept {

}

StatisticsSenderInfluxDb::StatisticsSenderInfluxDb(): httpclient__{new CurlHttpClient} {

};


}
