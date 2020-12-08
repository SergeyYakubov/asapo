#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>


#include "../src/single_folder_watch_windows.h"
#include "../src/event_monitor_error.h"
#include "../src/common.h"

#include "asapo/preprocessor/definitions.h"

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockLogger.h"

#include "mock_watch_io.h"

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
using asapo::SingleFolderWatch;
using asapo::FileInfos;
using asapo::FileInfo;

namespace {


TEST(SingleFolderWatch, Constructor) {
    SingleFolderWatch watch{"", "", nullptr};
    ASSERT_THAT(dynamic_cast<asapo::WatchIO*>(watch.watch_io__.get()), Ne(nullptr));
}

class SingleFolderWatchTests : public testing::Test {
  public:
    Error err;
    ::testing::NiceMock<asapo::MockWatchIO> mock_watch_io;
    testing::NiceMock<asapo::MockLogger> mock_logger;
    std::string expected_root_folder = "c:\\tmp";
    std::string expected_folder{"test1"};
    HANDLE expected_handle = HANDLE(1);
    asapo::SharedEventList event_list;
    SingleFolderWatch watch{expected_root_folder, expected_folder, &event_list};
    char* buffer;
    DWORD cur_buffer_pointer = 0;
    void SetUp() override {
        watch.watch_io__ = std::unique_ptr<asapo::WatchIO> {&mock_watch_io};
        watch.log__ = &mock_logger;
        buffer = new (char[asapo::kBufLen]);
    }
    void TearDown() override {
        watch.watch_io__.release();
        delete[] buffer;
    }
    void ExpectInit();
    void ExpectRead();
    void ExpectDirectory(bool yes);
    DWORD AddEventToBuffer(std::string filename, DWORD action);

};

DWORD SingleFolderWatchTests::AddEventToBuffer(std::string filename, DWORD action) {
    size_t filename_size = filename.size();
    DWORD size = sizeof(FILE_NOTIFY_INFORMATION) + filename_size * sizeof(WCHAR);
    char* buf = (char*) malloc(size);
    FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*) buf;
    event->NextEntryOffset = size;
    event->Action = action;
    for (size_t i = 0; i < filename_size; i++) {
        event->FileName[i] = filename[i];
    }
    event->FileNameLength = filename_size * sizeof(WCHAR);
    memcpy(buffer + cur_buffer_pointer, event, size);
    cur_buffer_pointer += size;
    free(buf);
    return size;
}

ACTION_P(A_CopyBuf, buffer) {
    memcpy(arg1, buffer, asapo::kBufLen);
}


void SingleFolderWatchTests::ExpectRead() {
    EXPECT_CALL(mock_watch_io, ReadDirectoryChanges_t(expected_handle, _, asapo::kBufLen, _))
    .WillOnce(DoAll(
                  A_CopyBuf(buffer),
                  SetArgPointee<3>(cur_buffer_pointer),
                  Return(nullptr))
             );
}


void SingleFolderWatchTests::ExpectInit() {
    EXPECT_CALL(mock_watch_io, Init_t(StrEq(expected_root_folder + asapo::kPathSeparator + expected_folder), _)).
    WillOnce(DoAll(
                 SetArgPointee<1>(nullptr),
                 Return(expected_handle)
             )
            );


}
void SingleFolderWatchTests::ExpectDirectory(bool yes) {
    EXPECT_CALL(mock_watch_io, IsDirectory(_)).
    WillRepeatedly(Return(yes)
                  );

}

TEST_F(SingleFolderWatchTests, InitWatchOnWatch) {
    ExpectInit();
    watch.Watch();
}

TEST_F(SingleFolderWatchTests, InitErrorOnWatch) {
    EXPECT_CALL(mock_watch_io, Init_t(StrEq(expected_root_folder + asapo::kPathSeparator + expected_folder), _)).
    WillOnce(DoAll(
                 SetArgPointee<1>(asapo::IOErrorTemplates::kFileNotFound.Generate().release()),
                 Return(INVALID_HANDLE_VALUE)
             )
            );

    EXPECT_CALL(mock_logger, Error(AllOf(
                                       HasSubstr("cannot add"),
                                       HasSubstr(expected_root_folder),
                                       HasSubstr(expected_folder),
                                       HasSubstr("file")
                                   )
                                  ));

    watch.Watch();
}

TEST_F(SingleFolderWatchTests, WatchWaitsBeforeEventIsAvailable) {
    ExpectInit();
    AddEventToBuffer("test", FILE_ACTION_MODIFIED);
    ExpectDirectory(false);
    ExpectRead();
    watch.Watch();

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    auto files = event_list.GetAndClearEvents();

    ASSERT_THAT(files.size(), Eq(0));
}

TEST_F(SingleFolderWatchTests, NewEventClearsTimeoutCounter) {
    ExpectInit();
    AddEventToBuffer("test", FILE_ACTION_MODIFIED);
    AddEventToBuffer("test2", FILE_ACTION_MODIFIED);
    ExpectDirectory(false);
    ExpectRead();
    watch.Watch();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    Mock::VerifyAndClearExpectations(&mock_watch_io);

    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    cur_buffer_pointer = 0;
    AddEventToBuffer("test2", FILE_ACTION_MODIFIED);
    ExpectRead();
    watch.Watch();

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    auto files = event_list.GetAndClearEvents();

    ASSERT_THAT(files.size(), Eq(1));
    ASSERT_THAT(files[0], StrEq(expected_folder + "\\test"));
}



TEST_F(SingleFolderWatchTests, WatchReadsDirectoryEventsAfterTimeout) {
    ExpectInit();
    AddEventToBuffer("test", FILE_ACTION_MODIFIED);
    AddEventToBuffer("test2", FILE_ACTION_MODIFIED);
    AddEventToBuffer("test2", FILE_ACTION_MODIFIED);
    ExpectDirectory(false);
    ExpectRead();
    watch.Watch();

    std::this_thread::sleep_for(std::chrono::milliseconds(asapo::kFileDelayMs + 10));
    auto files = event_list.GetAndClearEvents();

    ASSERT_THAT(files.size(), Eq(2));
    ASSERT_THAT(files[0], StrEq(expected_folder + "\\test"));
    ASSERT_THAT(files[1], StrEq(expected_folder + "\\test2"));


}


TEST_F(SingleFolderWatchTests, DirectoriesAreIgnored) {
    ExpectInit();
    AddEventToBuffer("test", FILE_ACTION_MODIFIED);
    ExpectDirectory(true);
    ExpectRead();
    watch.Watch();

    std::this_thread::sleep_for(std::chrono::milliseconds(asapo::kFileDelayMs + 10));
    auto files = event_list.GetAndClearEvents();

    ASSERT_THAT(files.size(), Eq(0));
}


TEST_F(SingleFolderWatchTests, OtherEventTypesAreIgnored) {
    ExpectInit();
    AddEventToBuffer("test1", FILE_ACTION_ADDED);
    AddEventToBuffer("test2", FILE_ACTION_REMOVED);
    AddEventToBuffer("test3", FILE_ACTION_RENAMED_OLD_NAME);
    ExpectDirectory(false);
    ExpectRead();
    watch.Watch();

    std::this_thread::sleep_for(std::chrono::milliseconds(asapo::kFileDelayMs + 10));
    auto files = event_list.GetAndClearEvents();

    ASSERT_THAT(files.size(), Eq(0));
}

TEST_F(SingleFolderWatchTests, NoWaitOnRenameEvent) {
    ExpectInit();
    AddEventToBuffer("test", FILE_ACTION_RENAMED_NEW_NAME);
    ExpectDirectory(false);
    ExpectRead();
    watch.Watch();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto files = event_list.GetAndClearEvents();

    ASSERT_THAT(files.size(), Eq(1));
}



}