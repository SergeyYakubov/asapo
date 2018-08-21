#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>

#include "../src/foldermon_config.h"
#include "../src/foldermon_config_factory.h"
#include "mock_foldermon_config.h"

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

using ::asapo::FolderMonConfigFactory;
using asapo::FolderMonConfig;

namespace {


class ConfigTests : public Test {
  public:
    MockIO mock_io;
    FolderMonConfigFactory config_factory;
    void SetUp() override {
        config_factory.io__ = std::unique_ptr<asapo::IO> {&mock_io};
    }
    void TearDown() override {
        config_factory.io__.release();
    }

};


TEST_F(ConfigTests, ReadSettingsOK) {
    asapo::FolderMonConfig test_config;
    test_config.nthreads = 10;
    test_config.tag = "folderMon1";
    test_config.log_level = asapo::LogLevel::Error;
    test_config.beamtime_id = "test";
    test_config.asapo_endpoint = "uri:001";
    test_config.mode = asapo::RequestHandlerType::kTcp;

    auto err = asapo::SetFolderMonConfig(test_config);

    auto config = asapo::GetFolderMonConfig();

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(config->log_level, Eq(asapo::LogLevel::Error));
    ASSERT_THAT(config->tag, Eq("folderMon1"));
    ASSERT_THAT(config->nthreads, Eq(10));
    ASSERT_THAT(config->beamtime_id, Eq("test"));
    ASSERT_THAT(config->asapo_endpoint, Eq("uri:001"));
    ASSERT_THAT(config->mode, Eq(asapo::RequestHandlerType::kTcp));
}

TEST_F(ConfigTests, ReadSettingsChecksNthreads) {
    asapo::FolderMonConfig test_config;
    test_config.nthreads = 0;

    auto err = asapo::SetFolderMonConfig(test_config);
    ASSERT_THAT(err, Ne(nullptr));

    test_config.nthreads = asapo::kMaxProcessingThreads + 1;

    err = asapo::SetFolderMonConfig(test_config);
    ASSERT_THAT(err, Ne(nullptr));

}


TEST_F(ConfigTests, ReadSettingsChecksMode) {
    asapo::FolderMonConfig test_config;
    test_config.nthreads = 1;
    test_config.asapo_endpoint = "wrongmode"; // we use it to set mode string to some wrong value
    auto err = asapo::SetFolderMonConfig(test_config);
    ASSERT_THAT(err, Ne(nullptr));
}


}