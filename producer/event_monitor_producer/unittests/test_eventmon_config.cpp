#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <asapo/unittests/MockIO.h>

#include "../src/eventmon_config.h"
#include "../src/eventmon_config_factory.h"
#include "mock_eventmon_config.h"

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
using testing::ElementsAre;

using ::asapo::Error;
using ::asapo::ErrorInterface;
using ::asapo::FileDescriptor;
using ::asapo::SocketDescriptor;
using ::asapo::MockIO;

using ::asapo::EventMonConfigFactory;
using asapo::EventMonConfig;

using asapo::SubSetMode;

namespace {


class ConfigTests : public Test {
  public:
    MockIO mock_io;
    EventMonConfigFactory config_factory;
    void SetUp() override {
        config_factory.io__ = std::unique_ptr<asapo::IO> {&mock_io};
    }
    void TearDown() override {
        config_factory.io__.release();
    }

};


TEST_F(ConfigTests, ReadSettingsOK) {
    asapo::EventMonConfig test_config;
    test_config.nthreads = 10;
    test_config.tag = "folderMon1";
    test_config.log_level = asapo::LogLevel::Error;
    test_config.beamtime_id = "test";
    test_config.asapo_endpoint = "uri:001";
    test_config.mode = asapo::RequestHandlerType::kTcp;
    test_config.root_monitored_folder = "tmp";
    test_config.monitored_subfolders = {"test1", "test2"};
    test_config.ignored_extensions = {"tmp", "test"};
    test_config.remove_after_send = true;
    test_config.subset_mode = SubSetMode::kBatch;
    test_config.subset_batch_size = 9;
    test_config.stream = "stream";
    test_config.whitelisted_extensions =  {"bla"};

    auto err = asapo::SetFolderMonConfig(test_config);

    auto config = asapo::GetEventMonConfig();

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(config->log_level, Eq(asapo::LogLevel::Error));
    ASSERT_THAT(config->tag, Eq("folderMon1"));
    ASSERT_THAT(config->nthreads, Eq(10));
    ASSERT_THAT(config->beamtime_id, Eq("test"));
    ASSERT_THAT(config->asapo_endpoint, Eq("uri:001"));
    ASSERT_THAT(config->mode, Eq(asapo::RequestHandlerType::kTcp));
    ASSERT_THAT(config->monitored_subfolders, ElementsAre("test1", "test2"));
    ASSERT_THAT(config->root_monitored_folder, Eq("tmp"));
    ASSERT_THAT(config->ignored_extensions, ElementsAre("tmp", "test"));
    ASSERT_THAT(config->remove_after_send, Eq(true));
    ASSERT_THAT(config->subset_mode, Eq(SubSetMode::kBatch));
    ASSERT_THAT(config->subset_batch_size, Eq(9));
    ASSERT_THAT(config->stream, Eq("stream"));
}


TEST_F(ConfigTests, ReadSettingsWhiteListOK) {
    asapo::EventMonConfig test_config;
    test_config.whitelisted_extensions =  {"tmp", "test"};
    test_config.ignored_extensions = {};

    auto err = asapo::SetFolderMonConfig(test_config);

    auto config = asapo::GetEventMonConfig();

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(config->whitelisted_extensions,  ElementsAre("tmp", "test"));
}

TEST_F(ConfigTests, ReadSettingsMultiSourceOK) {
    asapo::EventMonConfig test_config;
    test_config.subset_mode = SubSetMode::kMultiSource;
    test_config.subset_multisource_nsources = 2;
    test_config.subset_multisource_sourceid = 12;
    auto err = asapo::SetFolderMonConfig(test_config);

    auto config = asapo::GetEventMonConfig();

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(config->subset_mode, Eq(SubSetMode::kMultiSource));
    ASSERT_THAT(config->subset_multisource_nsources, Eq(2));
    ASSERT_THAT(config->subset_multisource_sourceid, Eq(12));

}





TEST_F(ConfigTests, ReadSettingsChecksNthreads) {
    asapo::EventMonConfig test_config;
    test_config.nthreads = 0;

    auto err = asapo::SetFolderMonConfig(test_config);
    ASSERT_THAT(err, Ne(nullptr));

    test_config.nthreads = asapo::kMaxProcessingThreads + 1;

    err = asapo::SetFolderMonConfig(test_config);
    ASSERT_THAT(err, Ne(nullptr));

}

TEST_F(ConfigTests, ReadSettingsChecksSubsets) {
    asapo::EventMonConfig test_config;
    test_config.subset_mode = SubSetMode::kBatch;
    test_config.subset_batch_size = 0;

    auto err = asapo::SetFolderMonConfig(test_config);
    ASSERT_THAT(err, Ne(nullptr));

    test_config.subset_mode = SubSetMode::kMultiSource;
    test_config.subset_multisource_nsources = 0;

    err = asapo::SetFolderMonConfig(test_config);
    ASSERT_THAT(err, Ne(nullptr));


}

TEST_F(ConfigTests, ReadSettingsDoesnotChecksSubsetsIfNoSubsets) {
    asapo::EventMonConfig test_config;
    test_config.subset_batch_size = 0;

    auto err = asapo::SetFolderMonConfig(test_config);
    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(ConfigTests, ReadSettingsChecksMode) {
    asapo::EventMonConfig test_config;
    test_config.nthreads = 1;
    test_config.asapo_endpoint = "wrongmode"; // we use it to set mode string to some wrong value
    auto err = asapo::SetFolderMonConfig(test_config);
    ASSERT_THAT(err, Ne(nullptr));
}


}
