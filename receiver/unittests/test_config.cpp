#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>

#include "../src/receiver_config.h"
#include "../src/receiver_config_factory.h"

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


TEST_F(ConfigTests, ErrorWaitForNewRequest) {

}

}
