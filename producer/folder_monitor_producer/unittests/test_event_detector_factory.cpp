#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "foldermon_mocking.h"
#include "mock_foldermon_config.h"
#include "../src/event_detector_factory.h"
#include "../src/inotify_event_detector.h"

using ::testing::Test;
using ::testing::_;
using ::testing::Eq;
using ::testing::Ne;

using ::asapo::Error;


using asapo::EventDetectorFactory;
using asapo::FolderMonConfig;

namespace {


class FactoryTests : public Test {
  public:
    EventDetectorFactory factory;
    Error err{nullptr};
    FolderMonConfig config;
    void SetUp() override {
        asapo::SetFolderMonConfig(config);
    }
    void TearDown() override {
    }
};

TEST_F(FactoryTests, CreateDetector) {
    auto event_detector = factory.CreateEventDetector();
    ASSERT_THAT(dynamic_cast<asapo::InotifyEventDetector*>(event_detector.get()), Ne(nullptr));
}

}
