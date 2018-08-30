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
using asapo::FilesToSend;
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
    std::vector<std::string> expected_subfolders2{"/tmp/test2/sub21", "/tmp/test2/sub22", "/tmp/test2/sub21/sub211"};
    std::vector<std::string> expected_watches{"/tmp/test1", "/tmp/test2", "/tmp/test1/sub11", "/tmp/test2/sub21", "/tmp/test2/sub22", "/tmp/test2/sub21/sub211"};
    std::string expected_filename1{"file1"};
    std::string expected_filename2{"file2"};

    int expected_wd = 10;
    std::vector<int>expected_fds = {1, 2, 3, 4, 5, 6};
    void MockStartMonitoring();
    char buffer[asapo::kBufLen]  __attribute__ ((aligned(8)));
    int cur_buffer_pointer;
    void SetUp() override {
        cur_buffer_pointer = 0;
        watch.inotify__ = std::unique_ptr<asapo::Inotify> {&mock_inotify};
        watch.io__ = std::unique_ptr<asapo::IO> {&mock_io};
    }
    void TearDown() override {
        watch.inotify__.release();
        watch.io__.release();
    }
    ssize_t  AddEventToBuffer(std::string filename, uint32_t mask, int fd);
    void ExpectRead();
    void ExpectCreateFolder(std::string folder);
};

void SystemFolderWatchTests::MockStartMonitoring() {
    EXPECT_CALL(mock_inotify, Init())
    .WillOnce(
        Return(expected_wd)
    );
    EXPECT_CALL(mock_io, GetSubDirectories_t(expected_root_folder + "/" + expected_folders[0], _))
    .WillOnce(
        Return(expected_subfolders1)
    );
    EXPECT_CALL(mock_io, GetSubDirectories_t(expected_root_folder + "/" + expected_folders[1], _))
    .WillOnce(
        Return(expected_subfolders2)
    );

    int i = 0;
    for (auto& watch  : expected_watches) {
        EXPECT_CALL(mock_inotify, AddWatch(expected_wd, StrEq(watch), asapo::kInotifyWatchFlags))
        .WillOnce(
            Return(expected_fds[i])
        );
        i++;
    }

    auto err = watch.StartFolderMonitor(expected_root_folder, expected_folders);
    ASSERT_THAT(err, Eq(nullptr));


    Mock::VerifyAndClearExpectations(&mock_inotify);
}

ACTION_P(A_CopyBuf, buffer) {
    memcpy(arg1, buffer, asapo::kBufLen);
}


ssize_t SystemFolderWatchTests::AddEventToBuffer(std::string filename, uint32_t mask, int fd) {
    struct inotify_event event;
    event.mask = mask;
    event.wd = fd;
    strcpy(event.name, filename.c_str());
    event.len = strlen(event.name) + 1;
    ssize_t size = sizeof(struct inotify_event) + event.len;
    memcpy(buffer + cur_buffer_pointer, &event, size);
    cur_buffer_pointer += size;
    return size;

}
void SystemFolderWatchTests::ExpectRead() {
    EXPECT_CALL(mock_inotify, Read(expected_wd, _, asapo::kBufLen))
    .WillOnce(DoAll(
                  A_CopyBuf(buffer),
                  Return(cur_buffer_pointer))
             );
}



TEST_F(SystemFolderWatchTests, ErrorInitInotifyStartMonitoring) {
    EXPECT_CALL(mock_inotify, Init())
    .WillOnce(
        Return(-1)
    );

    auto err = watch.StartFolderMonitor(expected_root_folder, expected_folders);
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
    EXPECT_CALL(mock_io, GetSubDirectories_t(expected_root_folder + "/" + expected_folders[0], _))
    .WillOnce(DoAll(
                  SetArgPointee<1>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(asapo::SubDirList{}))
             );

    auto err = watch.StartFolderMonitor(expected_root_folder, expected_folders);
    ASSERT_THAT(err, Eq(asapo::EventMonitorErrorTemplates::kSystemError));
}



TEST_F(SystemFolderWatchTests, ErrorAddSubfolderWatchStartMonitoring) {
    EXPECT_CALL(mock_inotify, Init())
    .WillOnce(
        Return(expected_wd)
    );

    EXPECT_CALL(mock_io, GetSubDirectories_t(expected_root_folder + "/" + expected_folders[0], _))
    .WillOnce(
        Return(expected_subfolders1)
    );
    EXPECT_CALL(mock_inotify, AddWatch(expected_wd, StrEq(expected_watches[0]), asapo::kInotifyWatchFlags))
    .WillOnce(
        Return(1)
    );

    EXPECT_CALL(mock_inotify, AddWatch(expected_wd, StrEq(expected_watches[2]), asapo::kInotifyWatchFlags))
    .WillOnce(
        Return(-1)
    );

    auto err = watch.StartFolderMonitor(expected_root_folder, expected_folders);

    ASSERT_THAT(err, Eq(asapo::EventMonitorErrorTemplates::kSystemError));
}


TEST_F(SystemFolderWatchTests, ErrorAddRootWatchStartMonitoring) {
    EXPECT_CALL(mock_inotify, Init())
    .WillOnce(
        Return(expected_wd)
    );

    EXPECT_CALL(mock_inotify, AddWatch(expected_wd, StrEq(expected_watches[0]), asapo::kInotifyWatchFlags))
    .WillOnce(
        Return(-1)
    );

    auto err = watch.StartFolderMonitor(expected_root_folder, expected_folders);

    ASSERT_THAT(err, Eq(asapo::EventMonitorErrorTemplates::kSystemError));
}

TEST_F(SystemFolderWatchTests, ErrorReadingEvents) {
    MockStartMonitoring();
    EXPECT_CALL(mock_inotify, Read(expected_wd, _, asapo::kBufLen))
    .WillOnce(
        Return(-1)
    );
    Error err;
    auto events = watch.GetFileList(&err);
    ASSERT_THAT(err, Eq(asapo::EventMonitorErrorTemplates::kSystemError));
}

TEST_F(SystemFolderWatchTests, ProcessFileEvents) {
    MockStartMonitoring();
    AddEventToBuffer(expected_filename1, IN_CLOSE_WRITE, expected_fds[0]);
    AddEventToBuffer(expected_filename2, IN_MOVED_TO, expected_fds[1]);
    ExpectRead();

    Error err;
    auto events = watch.GetFileList(&err);

    ASSERT_THAT(events.size(), Eq(2));
    ASSERT_THAT(events[0], StrEq("test1/file1"));
    ASSERT_THAT(events[1], StrEq("test2/file2"));
    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(SystemFolderWatchTests, ProcessFileEventsFolderNotFound) {
    MockStartMonitoring();
    AddEventToBuffer(expected_filename1, IN_CLOSE_WRITE, 100);
    ExpectRead();

    Error err;
    auto events = watch.GetFileList(&err);

    ASSERT_THAT(err, Eq(asapo::EventMonitorErrorTemplates::kSystemError));
}

TEST_F(SystemFolderWatchTests, ProcessDeleteFolder) {
    MockStartMonitoring();
    AddEventToBuffer("folder", IN_DELETE_SELF, expected_fds[0]);
    ExpectRead();
    EXPECT_CALL(mock_inotify, DeleteWatch(expected_fds[0], expected_wd));

    Error err;
    auto events = watch.GetFileList(&err);

    ASSERT_THAT(events.size(), Eq(0));
    ASSERT_THAT(err, Eq(nullptr));
}

void SystemFolderWatchTests::ExpectCreateFolder(std::string folder) {
    std::string newfolder = expected_root_folder + "/" + expected_folders[0] + "/" + folder;
    EXPECT_CALL(mock_io, GetSubDirectories_t(newfolder, _))
    .WillOnce(
        Return(std::vector<std::string> {newfolder + "/subfolder"})
    );
    EXPECT_CALL(mock_inotify, AddWatch(expected_wd, StrEq(newfolder), asapo::kInotifyWatchFlags))
    .WillOnce(
        Return(1)
    );
    EXPECT_CALL(mock_inotify, AddWatch(expected_wd, StrEq(newfolder + "/subfolder"), asapo::kInotifyWatchFlags))
    .WillOnce(
        Return(1)
    );
}

TEST_F(SystemFolderWatchTests, ProcessCreateFolder) {
    MockStartMonitoring();
    AddEventToBuffer("folder", IN_ISDIR | IN_CREATE, expected_fds[0]);
    ExpectRead();
    ExpectCreateFolder("folder");

    Error err;
    auto events = watch.GetFileList(&err);

    ASSERT_THAT(events.size(), Eq(0));
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(SystemFolderWatchTests, ProcessMoveFolder) {
    MockStartMonitoring();
    AddEventToBuffer("sub21", IN_ISDIR | IN_MOVED_TO, expected_fds[0]);
    AddEventToBuffer("sub21", IN_ISDIR | IN_MOVED_FROM, expected_fds[1]);
    ExpectRead();
    ExpectCreateFolder("sub21");
    EXPECT_CALL(mock_inotify, DeleteWatch(expected_fds[3], expected_wd));
    EXPECT_CALL(mock_inotify, DeleteWatch(expected_fds[5], expected_wd));

    Error err;
    auto events = watch.GetFileList(&err);

    ASSERT_THAT(events.size(), Eq(0));
    ASSERT_THAT(err, Eq(nullptr));
}


}