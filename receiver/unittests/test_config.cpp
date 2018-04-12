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
using ::hidra2::Error;
using ::hidra2::ErrorInterface;
using ::hidra2::FileDescriptor;
using ::hidra2::SocketDescriptor;
using ::hidra2::MockIO;

using ::hidra2::ReceiverConfigFactory;
using hidra2::GetReceiverConfig;

namespace {


class ConfigTests : public Test {
  public:
    MockIO mock_io;
    ReceiverConfigFactory config_factory;
    void SetUp() override {
        config_factory.io__ = std::unique_ptr<hidra2::IO> {&mock_io};
    }
    void TearDown() override {
        config_factory.io__.release();
    }

};


TEST_F(ConfigTests, ReadSettings) {

    hidra2::ReceiverConfig test_config;
    test_config.listen_port = 4200;
    test_config.monitor_db_name = "db_test";
    test_config.monitor_db_uri = "localhost:8086";
    test_config.write_to_disk = true;


    auto err = hidra2::SetReceiverConfig(test_config);

    auto config = GetReceiverConfig();

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(config->monitor_db_uri, Eq("localhost:8086"));
    ASSERT_THAT(config->monitor_db_name, Eq("db_test"));
    ASSERT_THAT(config->listen_port, Eq(4200));
    ASSERT_THAT(config->write_to_disk, true);

}

}
