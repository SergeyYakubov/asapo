#include "statistics_sender_influx_db.h"

#include <iostream>

#include "statistics.h"
#include "http_client/curl_http_client.h"
#include "receiver_config.h"

namespace hidra2 {

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args ) {
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
    std::unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 );
}


void StatisticsSenderInfluxDb::SendStatistics(const StatisticsToSend& statistic) const noexcept {
    HttpCode code;
    Error err;
    auto responce = httpclient__->Post(GetReceiverConfig()->monitor_db_uri + "/write?db=" +
                                       GetReceiverConfig()->monitor_db_name, StatisticsToString(statistic),
                                       &code, &err);
    if (err) {
        std::cerr << "Error sending statistics: " << err << std::endl;
        return;
    }

    if (code != HttpCode::OK && code != HttpCode::NoContent) {
        std::cerr << "Error sending statistics: " << responce << std::endl;
    }
}

std::string StatisticsSenderInfluxDb::StatisticsToString(const StatisticsToSend& statistic) const noexcept {
    std::string str;
    std::string tags = "receiver=1,connection=1";
    str = "statistics," + tags + " elapsed_ms=" + string_format("%ld", statistic.elapsed_ms);
    str += ",data_volume=" + string_format("%ld", statistic.data_volume);
    str += ",n_requests=" + string_format("%ld", statistic.n_requests);
    str += ",db_share=" + string_format("%.4f", statistic.entity_shares[StatisticEntity::kDatabase]);
    str += ",network_share=" + string_format("%.4f", statistic.entity_shares[StatisticEntity::kNetwork]);
    str += ",disk_share=" + string_format("%.4f", statistic.entity_shares[StatisticEntity::kDisk]);
    return str;
}

StatisticsSenderInfluxDb::StatisticsSenderInfluxDb(): httpclient__{new CurlHttpClient} {
};


}
