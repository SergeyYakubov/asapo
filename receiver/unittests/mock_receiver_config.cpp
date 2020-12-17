#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock_receiver_config.h"
#include "../src/receiver_config_factory.h"

#include <asapo/unittests/MockIO.h>

using testing::_;
using testing::Return;
using testing::SetArgPointee;

namespace asapo {

std::string Key(std::string value, std::string error_field) {
    auto val = (value == error_field ? "error" : value);
    return "\"" + val + "\":";
}

Error SetReceiverConfig (const ReceiverConfig& config, std::string error_field) {
    MockIO mock_io;
    ReceiverConfigFactory config_factory;
    config_factory.io__ = std::unique_ptr<IO> {&mock_io};

    std::string log_level;
    switch (config.log_level) {
    case LogLevel::Error:
        log_level = "error";
        break;
    case LogLevel::Warning:
        log_level = "warning";
        break;
    case LogLevel::Info:
        log_level = "info";
        break;
    case LogLevel::Debug:
        log_level = "debug";
        break;
    case LogLevel::None:
        log_level = "none";
        break;
    }

    auto config_string = std::string("{") + Key("PerformanceDbServer",
                                                error_field) + "\"" + config.performance_db_uri + "\"";
    config_string += "," + Key("PerformanceDbName", error_field) + "\"" + config.performance_db_name + "\"";
    config_string += "," + Key("DatabaseServer", error_field) + "\"" + config.database_uri + "\"";
    config_string += "," + Key("DiscoveryServer", error_field) + "\"" + config.discovery_server + "\"";
    config_string += "," + Key("ListenPort", error_field) + std::to_string(config.listen_port);
    config_string += "," + Key("DataServer", error_field) + "{";
    config_string += Key("ListenPort", error_field) + std::to_string(config.dataserver.listen_port);
    config_string += "," +  Key("AdvertiseURI", error_field) + "\"" + config.dataserver.advertise_uri + "\"";
    config_string += "," +  Key("NetworkMode", error_field);

    config_string += "[";
    for (auto it = config.dataserver.network_mode.begin() ; it != config.dataserver.network_mode.end(); ++it) {
        if (it != config.dataserver.network_mode.begin()) {
            config_string += ",";
        }
        config_string += "\"" + (*it) + "\"";
    }
    config_string += "]";

    config_string += "," + Key("NThreads", error_field) + std::to_string(config.dataserver.nthreads);
    config_string += "}";
    config_string += "," + Key("DataCache", error_field) + "{";
    config_string += Key("Use", error_field) + (config.use_datacache ? "true" : "false") ;
    config_string += "," + Key("SizeGB", error_field) + std::to_string(config.datacache_size_gb);
    config_string += "," + Key("ReservedShare", error_field) + std::to_string(config.datacache_reserved_share);
    config_string += "}";
    config_string += "," +  Key("AuthorizationInterval", error_field) + std::to_string(config.authorization_interval_ms);
    config_string += "," +  Key("ReceiveToDiskThresholdMB",
                                error_field) + std::to_string(config.receive_to_disk_threshold_mb);
    config_string += "," +  Key("AuthorizationServer", error_field) + "\"" + config.authorization_server + "\"";
    config_string += "," +  Key("LogLevel", error_field) + "\"" + log_level + "\"";
    config_string += "," +  Key("Tag", error_field) + "\"" + config.tag + "\"";
    config_string += "}";


    EXPECT_CALL(mock_io, ReadFileToString_t("fname", _)).WillOnce(
        testing::Return(config_string)
    );

    auto err = config_factory.SetConfig("fname");

    config_factory.io__.release();

    return err;
}

}


