#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/single_folder_watch_windows.h"
#include "../src/event_monitor_error.h"
#include "../src/common.h"

#include "preprocessor/definitions.h"

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"

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
    SingleFolderWatch watch{"",""};
    ASSERT_THAT(dynamic_cast<asapo::WatchIO*>(watch.watch_io__.get()), Ne(nullptr));
}

FileInfos CreateTestFileInfos() {
    FileInfos file_infos;
    FileInfo fi;
    fi.size = 100;
    fi.name = "file1";
    file_infos.push_back(fi);
    fi.name = "subfolder/file2";
    file_infos.push_back(fi);
    return file_infos;
}


class SingleFolderWatchTests : public testing::Test {
  public:
    Error err;
    ::testing::NiceMock<asapo::MockWatchIO> mock_watch_io;
    testing::NiceMock<asapo::MockLogger> mock_logger;
    std::string expected_root_folder = "c:\\tmp";
    std::string expected_folder{"test1"};
    HANDLE expected_handle = HANDLE(1);
    SingleFolderWatch watch{expected_root_folder,expected_folder};
    void SetUp() override {
        watch.watch_io__ = std::unique_ptr<asapo::WatchIO> {&mock_watch_io};
        watch.log__ = &mock_logger;
    }
    void TearDown() override {
        watch.watch_io__.release();
    }
};


TEST_F(SingleFolderWatchTests, InitWatchOnWatch) {
    EXPECT_CALL(mock_watch_io, Init_t(StrEq(expected_root_folder+asapo::kPathSeparator+expected_folder),_)).
        WillOnce(DoAll(
        SetArgPointee<1>(nullptr),
        Return(expected_handle)
                 )
    );

    watch.Watch();
}

TEST_F(SingleFolderWatchTests, InitErrorOnWatch) {
    EXPECT_CALL(mock_watch_io, Init_t(StrEq(expected_root_folder+asapo::kPathSeparator+expected_folder),_)).
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




}