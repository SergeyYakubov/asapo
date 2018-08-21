#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "foldermon_mocking.h"
#include "mock_foldermon_config.h"

using ::testing::Test;
using ::testing::_;
using ::testing::Eq;
using ::testing::Ne;

using ::asapo::Error;


//using asapo::EventDetectorFactory;
using asapo::FolderMonConfig;

namespace {


class FactoryTests : public Test {
 public:
  //EventDetectorFactory factory;
  Error err{nullptr};
  FolderMonConfig config;
  std::string asapo_endpoint{"endpoint"};
  void SetUp() override {
      config.asapo_endpoint = asapo_endpoint;
      asapo::SetFolderMonConfig(config);
  }
  void TearDown() override {
  }
};

TEST_F(FactoryTests, ErrorOnWrongCode) {
//    ASSERT_THAT(err, Ne(nullptr));
}



TEST_F(FactoryTests, DoNotAddDiskWriterIfNotWanted) {
//    config.write_to_disk = false;
//    SetReceiverConfig(config);

//      ASSERT_THAT(err, Eq(nullptr));
//    ASSERT_THAT(request->GetListHandlers().size(), Eq(2));
//    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
//    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbWrite*>(request->GetListHandlers().back()), Ne(nullptr));
}


}
