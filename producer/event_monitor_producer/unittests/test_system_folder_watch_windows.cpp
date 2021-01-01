#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/system_folder_watch_windows.h"
#include "../src/event_monitor_error.h"
#include "../src/common.h"

#include <asapo/unittests/MockIO.h>



using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::SetArgPointee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::InSequence;
using ::testing::HasSubstr;
using testing::StrEq;

using ::asapo::Error;
using ::asapo::ErrorInterface;
using asapo::FilesToSend;
using asapo::SystemFolderWatch;
using asapo::MessageMetas;
using asapo::MessageMeta;

namespace {


TEST(SystemFolderWatch, Constructor) {
    SystemFolderWatch watch;
    ASSERT_THAT(dynamic_cast<asapo::IO*>(watch.io__.get()), Ne(nullptr));
}

MessageMetas CreateTestMessageMetas() {
    MessageMetas message_metas;
    MessageMeta fi;
    fi.size = 100;
    fi.name = "file1";
    message_metas.push_back(fi);
    fi.name = "subfolder\\file2";
    message_metas.push_back(fi);
    return message_metas;
}


class SystemFolderWatchTests : public testing::Test {
  public:
    Error err;
    ::testing::NiceMock<asapo::MockIO> mock_io;
    SystemFolderWatch watch{};
    std::string expected_root_folder = "c:\\tmp";
    std::vector<std::string> expected_folders{"test1", "test2"};
    void SetUp() override {
        watch.io__ = std::unique_ptr<asapo::IO> {&mock_io};
    }
    void TearDown() override {
        watch.io__.release();
    }
};

TEST_F(SystemFolderWatchTests, StartMonitoring) {


    EXPECT_CALL(mock_io, NewThread_t(_)).Times(expected_folders.size()).
    WillRepeatedly(
        Return(nullptr)
    );

    auto err = watch.StartFolderMonitor(expected_root_folder, expected_folders);
    ASSERT_THAT(err, Eq(nullptr));
}





}