#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "eventmon_mocking.h"
#include "mock_eventmon_config.h"
#include "../src/event_detector_factory.h"
#include "../src/inotify_event_detector.h"

using ::testing::Test;
using ::testing::_;
using ::testing::Eq;
using ::testing::Ne;

using ::asapo::Error;


using asapo::EventDetectorFactory;
using asapo::EventMonConfig;

namespace {


class FactoryTests : public Test {
  public:
    EventDetectorFactory factory;
    Error err{nullptr};
    EventMonConfig config;
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
