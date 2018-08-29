#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/system_folder_watch_linux.h"
#include "../src/event_monitor_error.h"
#include "../src/common.h"

#include "mock_inotify.h"
#include <unittests/MockIO.h>



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
using asapo::FileEvents;
using asapo::FileEvent;
using asapo::SystemFolderWatch;

namespace {


TEST(SystemFolderWatch, Constructor) {
    SystemFolderWatch watch;
    ASSERT_THAT(dynamic_cast<asapo::IO*>(watch.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::Inotify*>(watch.inotify__.get()), Ne(nullptr));
}

class SystemFolderWatchTests : public testing::Test {
 public:
  Error err;
  ::testing::NiceMock<asapo::MockInotify> mock_inotify;
  ::testing::NiceMock<asapo::MockIO> mock_io;
  SystemFolderWatch watch{};
  std::string expected_root_folder = "/tmp";
  std::vector<std::string> expected_folders{"test1", "test2"};
  std::vector<std::string> expected_subfolders1{"/tmp/test1/sub11"};
  std::vector<std::string> expected_subfolders2{"/tmp/test2/sub21","/tmp/test2/sub22"};
  std::vector<std::string> expected_watches{"/tmp/test1", "/tmp/test2","/tmp/test1/sub11","/tmp/test2/sub21","/tmp/test2/sub22"};
  int expected_wd = 1;
  int expected_fd = 1;
  void MockStartMonitoring();
  void SetUp() override {
      watch.inotify__ = std::unique_ptr<asapo::Inotify> {&mock_inotify};
      watch.io__ = std::unique_ptr<asapo::IO> {&mock_io};
  }
  void TearDown() override {
      watch.inotify__.release();
      watch.io__.release();
  }
};

void SystemFolderWatchTests::MockStartMonitoring() {
    EXPECT_CALL(mock_inotify, Init())
        .WillOnce(
            Return(expected_wd)
        );
    EXPECT_CALL(mock_io, GetSubDirectories_t(expected_root_folder+"/"+expected_folders[0],_))
        .WillOnce(
            Return(expected_subfolders1)
        );
    EXPECT_CALL(mock_io, GetSubDirectories_t(expected_root_folder+"/"+expected_folders[1],_))
        .WillOnce(
            Return(expected_subfolders2)
        );

    for (auto& watch  : expected_watches) {
        EXPECT_CALL(mock_inotify,AddWatch(expected_wd,StrEq(watch),asapo::kInotifyWatchFlags))
            .WillOnce(
                Return(expected_fd)
            );
    }

    auto err = watch.StartFolderMonitor(expected_root_folder,expected_folders);
    ASSERT_THAT(err, Eq(nullptr));


    Mock::VerifyAndClearExpectations(&mock_inotify);
}

TEST_F(SystemFolderWatchTests, ErrorInitInotifyStartMonitoring) {
    EXPECT_CALL(mock_inotify, Init())
        .WillOnce(
            Return(-1)
        );

    auto err = watch.StartFolderMonitor(expected_root_folder,expected_folders);
    ASSERT_THAT(err, Eq(asapo::EventMonitorErrorTemplates::kSystemError));
}


TEST_F(SystemFolderWatchTests, OKInitInotifyStartMonitoring) {
    MockStartMonitoring();
}


TEST_F(SystemFolderWatchTests, ErrorGetSubdirsStartMonitoring) {
    EXPECT_CALL(mock_inotify, Init())
        .WillOnce(
            Return(expected_wd)
        );
    EXPECT_CALL(mock_io, GetSubDirectories_t(expected_root_folder+"/"+expected_folders[0],_))
        .WillOnce(DoAll(
            SetArgPointee<1>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
            Return(asapo::SubDirList{}))
        );

    auto err = watch.StartFolderMonitor(expected_root_folder,expected_folders);
    ASSERT_THAT(err, Eq(asapo::EventMonitorErrorTemplates::kSystemError));
}



}


