#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>

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

namespace {


class ConfigTests : public Test {
  public:
    MockIO mock_io;
    void SetUp() override {
    }
    void TearDown() override {
    }

};


TEST_F(ConfigTests, ReadSettings) {
}

}
