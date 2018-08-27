#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/folder_event_detector.h"
#include "../src/event_monitor_error.h"
#include "../src/common.h"

#include "unittests/MockIO.h"

#include "mock_system_folder_watch.h"



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


using ::asapo::Error;
using ::asapo::FileDescriptor;
using ::asapo::ErrorInterface;
using asapo::FileEvents;
using asapo::FileEvent;
using asapo::EventType;
using asapo::FolderEventDetector;

namespace {


TEST(FolderEventDetector, Constructor) {
    asapo::EventMonConfig test_config;
    FolderEventDetector detector{&test_config};
    ASSERT_THAT(dynamic_cast<asapo::SystemFolderWatch*>(detector.system_folder_watch__.get()), Ne(nullptr));
}


class FolderEventDetectorTests : public testing::Test {
  public:
    Error err;
    ::testing::NiceMock<asapo::MockSystemFolderWatch> mock_system_folder_watch;
    asapo::EventMonConfig test_config;
    FolderEventDetector detector{&test_config};
    std::vector<std::string> expected_folders{"test1", "test2"};
    FileEvent expected_event1{EventType::closed, 10, "test1.dat"};
    FileEvent expected_event2{EventType::renamed_to, 11, "test2.dat"};
    FileEvent expected_event3{EventType::renamed_to, 11, "test3.tmp"};
    FileEvent expected_event4{EventType::closed, 12, "test4.tmp"};
    FileEvents expected_events{expected_event1, expected_event2, expected_event3, expected_event4};
    void SetUp() override {
        test_config.monitored_folders = expected_folders;
        err = nullptr;
        detector.system_folder_watch__ = std::unique_ptr<asapo::SystemFolderWatch> {&mock_system_folder_watch};
    }
    void TearDown() override {
        detector.system_folder_watch__.release();
    }
    void MockStartMonitoring();
    void MockGetEvents();
    void InitiateAndReadSingleEvent();
};

void FolderEventDetectorTests::MockStartMonitoring() {
    EXPECT_CALL(mock_system_folder_watch, StartFolderMonitor_t(expected_folders))
    .WillOnce(
        Return(nullptr)
    );
}



TEST_F(FolderEventDetectorTests, StartsFolderMonitoringOK) {
    EXPECT_CALL(mock_system_folder_watch, StartFolderMonitor_t(expected_folders))
    .WillOnce(
        Return(nullptr)
    );
    auto err = detector.StartMonitoring();
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(FolderEventDetectorTests, StartFolderMonitoringInitiatesOnlyOnce) {
    EXPECT_CALL(mock_system_folder_watch, StartFolderMonitor_t(expected_folders))
    .WillOnce(
        Return(nullptr)
    );
    auto err = detector.StartMonitoring();
    ASSERT_THAT(err, Eq(nullptr));
    err = detector.StartMonitoring();
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(FolderEventDetectorTests, StartFolderMonitoringReturnsError) {
    EXPECT_CALL(mock_system_folder_watch, StartFolderMonitor_t(expected_folders))
    .Times(2)
    .WillOnce(
        Return(asapo::ErrorTemplates::kMemoryAllocationError.Generate().release())
    )
    .WillOnce(
        Return(nullptr)
    )
    ;
    auto err = detector.StartMonitoring();
    ASSERT_THAT(err, Ne(nullptr));
    err = detector.StartMonitoring();
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(FolderEventDetectorTests, GetNextReturnsErrorIfMonitoringNotStarted) {
    auto err = detector.GetNextEvent(nullptr);
    ASSERT_THAT(err, Ne(nullptr));
}

TEST_F(FolderEventDetectorTests, GetNextCallsSystemGetNextFirstTimeNoEvents) {
    MockStartMonitoring();
    asapo::EventHeader event_header;
    EXPECT_CALL(mock_system_folder_watch, GetFileEventList_t(_));


    detector.StartMonitoring();

    auto err = detector.GetNextEvent(&event_header);
    ASSERT_THAT(err, Eq(asapo::EventMonitorErrorTemplates::kNoNewEvent));
}

TEST_F(FolderEventDetectorTests, GetNextEventError) {
    MockStartMonitoring();
    EXPECT_CALL(mock_system_folder_watch, GetFileEventList_t(_)).WillOnce(
        DoAll(
            SetArgPointee<0>(asapo::EventMonitorErrorTemplates::kSystemError.Generate().release()),
            Return(FileEvents{})
        ));

    detector.StartMonitoring();

    asapo::EventHeader event_header;
    auto err = detector.GetNextEvent(&event_header);
    ASSERT_THAT(err, Eq(asapo::EventMonitorErrorTemplates::kSystemError));
}

void FolderEventDetectorTests::MockGetEvents() {
    EXPECT_CALL(mock_system_folder_watch, GetFileEventList_t(_)).WillOnce(
        DoAll(
            SetArgPointee<0>(nullptr),
            Return(expected_events)
        ));
}

void FolderEventDetectorTests::InitiateAndReadSingleEvent() {
    MockStartMonitoring();
    MockGetEvents();
    detector.StartMonitoring();
    asapo::EventHeader event_header;
    detector.GetNextEvent(&event_header);
    Mock::VerifyAndClearExpectations(&mock_system_folder_watch);
};


TEST_F(FolderEventDetectorTests, GetNextEventOK) {
    MockStartMonitoring();
    MockGetEvents();

    detector.StartMonitoring();

    asapo::EventHeader event_header;
    auto err = detector.GetNextEvent(&event_header);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(event_header.file_name, Eq("test1.dat"));
    ASSERT_THAT(event_header.file_size, Eq(10));
}



TEST_F(FolderEventDetectorTests, GetNextEventDoesDoSystemCallIfListNotEmpty) {
    InitiateAndReadSingleEvent();

    EXPECT_CALL(mock_system_folder_watch, GetFileEventList_t(_)).Times(0);


    asapo::EventHeader event_header;
    auto err = detector.GetNextEvent(&event_header);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(event_header.file_name, Eq("test2.dat"));
    ASSERT_THAT(event_header.file_size, Eq(11));
}


TEST_F(FolderEventDetectorTests, GetNextEventDoesSystemCallIfListEmpty) {
    InitiateAndReadSingleEvent();
    EXPECT_CALL(mock_system_folder_watch, GetFileEventList_t(_)).Times(1);

// read events 2 to 4
    asapo::EventHeader event_header;
    err = detector.GetNextEvent(&event_header);
    ASSERT_THAT(err, Eq(nullptr));
    err = detector.GetNextEvent(&event_header);
    ASSERT_THAT(err, Eq(nullptr));
    err = detector.GetNextEvent(&event_header);
    ASSERT_THAT(err, Eq(nullptr));
// read events - should initiate system call since the buffer is empty now
    err = detector.GetNextEvent(&event_header);
    ASSERT_THAT(err, Eq(asapo::EventMonitorErrorTemplates::kNoNewEvent));
}

TEST_F(FolderEventDetectorTests, GetNextIgnoresTmpFiles) {
    test_config.ignored_extentions = {"tmp"};
    InitiateAndReadSingleEvent();
    asapo::EventHeader event_header;
    err = detector.GetNextEvent(&event_header);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(event_header.file_name, Eq("test2.dat"));

// try read event 3 test3.tmp sould be ignored
    err = detector.GetNextEvent(&event_header);
    ASSERT_THAT(err, Eq(asapo::EventMonitorErrorTemplates::kNoNewEvent));
}

}


