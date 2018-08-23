#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/folder_event_detector.h"

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"

#include "MockSystemFolderWatch.h"



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
  ::testing::NiceMock<asapo::MockLogger> mock_logger;
  ::testing::NiceMock<asapo::MockSystemFolderWatch> mock_system_folder_watch;
  asapo::EventMonConfig test_config;
  FolderEventDetector detector{&test_config};
  std::vector<std::string> expected_folders{"test1","test2"};
  void SetUp() override {
//      test_config.expected_folders = expected_folders;
      err = nullptr;
      detector.system_folder_watch__ = std::unique_ptr<asapo::SystemFolderWatch> {&mock_system_folder_watch};
      detector.log__ = &mock_logger;
  }
  void TearDown() override {
      detector.system_folder_watch__.release();
  }


};

TEST_F(FolderEventDetectorTests, GetNextEventCallsGetFolderEvents) {

//    EXPECT_CALL(mock_system_folder_watch, StartFolderMonitor_t(expected_folders, receiver.kMaxUnacceptedConnectionsBacklog, _))
//        .WillOnce(DoAll(
//            SetArgPointee<2>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
//            Return(0)
//        ));

    //EXPECT_CALL(mock_logger, Error(HasSubstr("prepare listener")));


//    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}

}


