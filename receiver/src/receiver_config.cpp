#include "receiver_config.h"
#include "receiver_config_factory.h"
#include "asapo/io/io_factory.h"
#include "asapo/json_parser/json_parser.h"

#include <iostream>

namespace asapo {

ReceiverConfig config;

ReceiverConfigFactory::ReceiverConfigFactory() : io__{GenerateDefaultIO()} {

}

Error ReceiverConfigFactory::SetConfig(std::string file_name) {
    JsonFileParser parser(file_name, &io__);
    std::string log_level;
    Error err;

    (err = parser.GetString("PerformanceDbServer", &config.performance_db_uri)) ||
    (err = parser.GetBool("MonitorPerformance", &config.monitor_performance)) ||
    (err = parser.GetUInt64("ListenPort", &config.listen_port)) ||
    (err = parser.GetUInt64("ReceiveToDiskThresholdMB", &config.receive_to_disk_threshold_mb)) ||
    (err = parser.Embedded("DataServer").GetUInt64("ListenPort", &config.dataserver.listen_port)) ||
    (err = parser.Embedded("DataServer").GetUInt64("NThreads", &config.dataserver.nthreads)) ||
    (err = parser.Embedded("DataCache").GetBool("Use", &config.use_datacache)) ||
    (err = parser.Embedded("DataCache").GetUInt64("SizeGB", &config.datacache_size_gb)) ||
    (err = parser.Embedded("DataCache").GetUInt64("ReservedShare", &config.datacache_reserved_share)) ||
    (err = parser.GetString("DatabaseServer", &config.database_uri)) ||
    (err = parser.GetString("DiscoveryServer", &config.discovery_server)) ||
    (err = parser.GetString("Tag", &config.tag)) ||
    (err = parser.GetString("AuthorizationServer", &config.authorization_server)) ||
    (err = parser.GetUInt64("AuthorizationInterval", &config.authorization_interval_ms)) ||
    (err = parser.GetString("PerformanceDbName", &config.performance_db_name)) ||
    (err = parser.Embedded("DataServer").GetString("AdvertiseURI", &config.dataserver.advertise_uri)) ||
    (err = parser.Embedded("DataServer").GetArrayString("NetworkMode", &config.dataserver.network_mode)) ||
    (err = parser.GetString("LogLevel", &log_level));

    if (err) {
        return err;
    }

    config.dataserver.tag = config.tag + "_ds";

    config.log_level = StringToLogLevel(log_level, &err);
    return err;

}

const ReceiverConfig* GetReceiverConfig() {
    return &config;
}


}
