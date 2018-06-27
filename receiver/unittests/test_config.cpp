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
    void SetUp() override {
        config_factory.io__ = std::unique_ptr<asapo::IO> {&mock_io};
    }
    void TearDown() override {
        config_factory.io__.release();
    }

};


TEST_F(ConfigTests, ReadSettings) {

    asapo::ReceiverConfig test_config;
    test_config.listen_port = 4200;
    test_config.tag = "receiver1";
    test_config.monitor_db_name = "db_test";
    test_config.monitor_db_uri = "localhost:8086";
    test_config.write_to_disk = true;
    test_config.write_to_db = true;
    test_config.broker_db_uri = "localhost:27017";
    test_config.log_level = asapo::LogLevel::Error;
    test_config.root_folder = "test_fodler";
    test_config.authorization_interval_ms = 10000;
    test_config.authorization_server = "AuthorizationServer";

    auto err = asapo::SetReceiverConfig(test_config);

    auto config = GetReceiverConfig();

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(config->monitor_db_uri, Eq("localhost:8086"));
    ASSERT_THAT(config->monitor_db_name, Eq("db_test"));
    ASSERT_THAT(config->broker_db_uri, Eq("localhost:27017"));
    ASSERT_THAT(config->listen_port, Eq(4200));
    ASSERT_THAT(config->authorization_interval_ms, Eq(10000));
    ASSERT_THAT(config->authorization_server, Eq("AuthorizationServer"));
    ASSERT_THAT(config->write_to_disk, Eq(true));
    ASSERT_THAT(config->write_to_db, Eq(true));
    ASSERT_THAT(config->log_level, Eq(asapo::LogLevel::Error));
    ASSERT_THAT(config->tag, Eq("receiver1"));
    ASSERT_THAT(config->root_folder, Eq("test_fodler"));

}

}
