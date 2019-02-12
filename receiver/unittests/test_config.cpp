#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>

#include "../src/receiver_config.h"
#include "../src/receiver_config_factory.h"
#include "mock_receiver_config.h"

using ::testing::Test;
using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::InSequence;
using ::testing::SetArgPointee;
using ::asapo::Error;
using ::asapo::ErrorInterface;
using ::asapo::FileDescriptor;
using ::asapo::SocketDescriptor;
using ::asapo::MockIO;

using ::asapo::ReceiverConfigFactory;
using asapo::GetReceiverConfig;

namespace {


class ConfigTests : public Test {
  public:
    MockIO mock_io;
    ReceiverConfigFactory config_factory;
    asapo::ReceiverConfig test_config;
    void SetUp() override {
        config_factory.io__ = std::unique_ptr<asapo::IO> {&mock_io};
    }
    void TearDown() override {
        config_factory.io__.release();
    }
    void PrepareConfig() {
        test_config.listen_port = 4200;
        test_config.dataserver_listen_port = 4201;
        test_config.tag = "receiver1";
        test_config.monitor_db_name = "db_test";
        test_config.monitor_db_uri = "localhost:8086";
        test_config.write_to_disk = true;
        test_config.write_to_db = true;
        test_config.broker_db_uri = "localhost:27017";
        test_config.log_level = asapo::LogLevel::Error;
        test_config.root_folder = "test_fodler";
        test_config.authorization_interval_ms = 10000;
        test_config.authorization_server = "AuthorizationServer/aa";
        test_config.use_datacache = false;
        test_config.datacache_reserved_share = 10;
        test_config.datacache_size_gb = 2;
    }

};


TEST_F(ConfigTests, ReadSettings) {
    PrepareConfig();

    auto err = asapo::SetReceiverConfig(test_config, "none");

    auto config = GetReceiverConfig();

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(config->monitor_db_uri, Eq("localhost:8086"));
    ASSERT_THAT(config->monitor_db_name, Eq("db_test"));
    ASSERT_THAT(config->broker_db_uri, Eq("localhost:27017"));
    ASSERT_THAT(config->listen_port, Eq(4200));
    ASSERT_THAT(config->dataserver_listen_port, Eq(4201));
    ASSERT_THAT(config->authorization_interval_ms, Eq(10000));
    ASSERT_THAT(config->authorization_server, Eq("AuthorizationServer/aa"));
    ASSERT_THAT(config->write_to_disk, Eq(true));
    ASSERT_THAT(config->write_to_db, Eq(true));
    ASSERT_THAT(config->log_level, Eq(asapo::LogLevel::Error));
    ASSERT_THAT(config->tag, Eq("receiver1"));
    ASSERT_THAT(config->root_folder, Eq("test_fodler"));
    ASSERT_THAT(config->use_datacache, Eq(false));
    ASSERT_THAT(config->datacache_reserved_share, Eq(10));
    ASSERT_THAT(config->datacache_size_gb, Eq(2));

}


TEST_F(ConfigTests, ErrorReadSettings) {
    PrepareConfig();

    std::vector<std::string>fields {"MonitorDbAddress", "ListenPort", "DataServer", "ListenPort", "WriteToDisk",
                                    "WriteToDb", "DataCache", "Use", "SizeGB", "ReservedShare", "BrokerDbAddress", "Tag",
                                    "AuthorizationServer", "AuthorizationInterval", "RootFolder", "MonitorDbName", "LogLevel"};
    for (const auto& field : fields) {
        auto err = asapo::SetReceiverConfig(test_config, field);
        ASSERT_THAT(err, Ne(nullptr));
    }
}


}
