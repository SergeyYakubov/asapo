#include "statistics_sender_influx_db.h"

#include <iostream>

#include "statistics.h"
#include "../receiver_config.h"
#include "../receiver_logger.h"

namespace asapo {

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args ) {
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
    std::unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 );
}


void StatisticsSenderInfluxDb::SendStatistics(const StatisticsToSend& statistic) const noexcept {
    //todo: send statistics async
    HttpCode code;
    Error err;
    auto response = httpclient__->Post(GetReceiverConfig()->performance_db_uri + "/write?db=" +
                                       GetReceiverConfig()->performance_db_name, "", StatisticsToString(statistic),
                                       &code, &err);
    std::string msg = "sending statistics to " + GetReceiverConfig()->performance_db_name + " at " +
                      GetReceiverConfig()->performance_db_uri;
    if (err) {
        log__->Error(msg + " - " + err->Explain());
        return;
    }

    if (code != HttpCode::OK && code != HttpCode::NoContent) {
        log__->Error(msg + " - " + response);
        return;
    }
}

std::string StatisticsSenderInfluxDb::StatisticsToString(const StatisticsToSend& statistic) const noexcept {
    std::string str, tags;
    for (auto tag : statistic.tags) {
        tags += "," + tag.first + "=" + tag.second;
    }
    str = "statistics" + tags + " elapsed_ms=" + string_format("%ld", statistic.elapsed_ms);
    str += ",data_volume=" + string_format("%ld", statistic.data_volume);
    str += ",n_requests=" + string_format("%ld", statistic.n_requests);
    for (const auto& entity : statistic.extra_entities) {
        str += "," + entity.first + "=" + string_format("%.4f", entity.second);
    }
    return str;
}

StatisticsSenderInfluxDb::StatisticsSenderInfluxDb(): httpclient__{DefaultHttpClient()}, log__{GetDefaultReceiverLogger()} {
    HttpCode code;
    Error err;
    auto response = httpclient__->Post(GetReceiverConfig()->performance_db_uri + "/query", "",
                                       "q=create database " + GetReceiverConfig()->performance_db_name, &code, &err);
    std::string msg = "initializing statistics for " + GetReceiverConfig()->performance_db_name + " at " +
                      GetReceiverConfig()->performance_db_uri;
    if (err) {
        log__->Warning(msg + " - " + err->Explain());
        return;
    }

    if (code != HttpCode::OK && code != HttpCode::NoContent) {
        log__->Warning(msg + " - " + response);
        return;
    }

    log__->Debug(msg);

};


}
