#include <gmock/gmock.h>

#include "worker/data_broker.h"
#include "../src/folder_data_broker.h"



using hidra2::DataBrokerFactory;
using hidra2::DataBroker;
using hidra2::FolderDataBroker;
using hidra2::WorkerErrorCode;

using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;

namespace {

class FolderDataBrokerTests : public Test {
  public:
    std::unique_ptr<FolderDataBroker> data_broker;
    void SetUp() override {
        data_broker = std::unique_ptr<FolderDataBroker> {new FolderDataBroker("/path/to/file")};
        data_broker->set_io_(nullptr);
    }
    void TearDown() override {
    }
};

TEST_F(FolderDataBrokerTests, CanConnect) {
    auto return_code = data_broker->Connect();
    ASSERT_THAT(return_code, Eq(WorkerErrorCode::ERR__NO_ERROR));
}

}
