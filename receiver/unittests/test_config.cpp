#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <asapo/unittests/MockIO.h>

#include "../src/receiver_config.h"
#include "mock_receiver_config.h"

using namespace testing;
using namespace asapo;

namespace {


class ConfigTests : public Test {
  public:
    MockIO mock_io;
    ReceiverConfigManager config_factory;
    asapo::ReceiverConfig test_config;
    void SetUp() override {
        config_factory.io__ = std::unique_ptr<asapo::IO> {&mock_io};
    }
    void TearDown() override {
        config_factory.io__.release();
    }
    void PrepareConfig() {
        test_config.listen_port = 4200;
        test_config.tag = "receiver1";
        test_config.performance_db_name = "db_test";
        test_config.performance_db_uri = "localhost:8086";
        test_config.monitor_performance = true;
        test_config.database_uri = "localhost:27017";
        test_config.log_level = asapo::LogLevel::Error;
        test_config.authorization_interval_ms = 10000;
        test_config.authorization_server = "AuthorizationServer/aa";
        test_config.use_datacache = false;
        test_config.datacache_reserved_share = 10;
        test_config.datacache_size_gb = 2;
        test_config.discovery_server = "discovery";
        test_config.dataserver.nthreads = 5;
        test_config.dataserver.listen_port = 4201;
        test_config.dataserver.advertise_uri = "0.0.0.1:4201";
        test_config.dataserver.network_mode = {"tcp", "fabric"};
        test_config.receive_to_disk_threshold_mb = 50;
        test_config.metrics.expose = true;
        test_config.metrics.listen_port = 123;


    }

};


TEST_F(ConfigTests, ReadSettings) {
    PrepareConfig();

    auto err = asapo::SetReceiverConfigWithError(test_config, "none");

    auto config = GetReceiverConfig();

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(config->performance_db_uri, Eq("localhost:8086"));
    ASSERT_THAT(config->performance_db_name, Eq("db_test"));
    ASSERT_THAT(config->database_uri, Eq("localhost:27017"));
    ASSERT_THAT(config->listen_port, Eq(4200));
    ASSERT_THAT(config->authorization_interval_ms, Eq(10000));
    ASSERT_THAT(config->authorization_server, Eq("AuthorizationServer/aa"));
    ASSERT_THAT(config->log_level, Eq(asapo::LogLevel::Error));
    ASSERT_THAT(config->tag, Eq("receiver1"));
    ASSERT_THAT(config->use_datacache, Eq(false));
    ASSERT_THAT(config->monitor_performance, Eq(true));
    ASSERT_THAT(config->datacache_reserved_share, Eq(10));
    ASSERT_THAT(config->datacache_size_gb, Eq(2));
    ASSERT_THAT(config->discovery_server, Eq("discovery"));
    ASSERT_THAT(config->dataserver.nthreads, Eq(5));
    ASSERT_THAT(config->dataserver.tag, Eq("receiver1_ds"));
    ASSERT_THAT(config->dataserver.listen_port, Eq(4201));
    ASSERT_THAT(config->dataserver.advertise_uri, Eq("0.0.0.1:4201"));
    ASSERT_THAT(config->dataserver.network_mode.size(), Eq(2));
    ASSERT_THAT(config->dataserver.network_mode[0], Eq("tcp"));
    ASSERT_THAT(config->dataserver.network_mode[1], Eq("fabric"));
    ASSERT_THAT(config->receive_to_disk_threshold_mb, Eq(50));
    ASSERT_THAT(config->metrics.expose, Eq(true));
    ASSERT_THAT(config->metrics.listen_port, Eq(123));
    ASSERT_THAT(config->kafka_config.enabled, Eq(false));

}



TEST_F(ConfigTests, ErrorReadSettings) {
    PrepareConfig();

    std::vector<std::string>fields {"PerformanceDbServer", "ListenPort", "DataServer", "ListenPort",
                                    "DataCache", "Use", "SizeGB", "ReservedShare", "DatabaseServer", "Tag",
                                    "AuthorizationServer", "AuthorizationInterval", "PerformanceDbName", "LogLevel",
                                    "NThreads", "DiscoveryServer", "AdvertiseURI", "NetworkMode", "MonitorPerformance",
                                    "ReceiveToDiskThresholdMB", "Metrics", "Expose","Enabled"};
    for (const auto& field : fields) {
        auto err = asapo::SetReceiverConfigWithError(test_config, field);
        ASSERT_THAT(err, Ne(nullptr));
    }
}


}
