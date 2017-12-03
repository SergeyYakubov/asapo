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

TEST(DataBrokerFactoryTests, CreateFolderDataSource) {
    WorkerErrorCode return_code;

    auto data_broker = DataBrokerFactory::Create("path/to/file", &return_code);

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::ERR__NO_ERROR));
    ASSERT_THAT(dynamic_cast<FolderDataBroker*>(data_broker.release()), Ne(nullptr));
}

TEST(DataBrokerFactoryTests, FailCreateDataSourceWithEmptySource) {
    WorkerErrorCode return_code;

    auto data_broker = DataBrokerFactory::Create("", &return_code);

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::ERR__EMPTY_DATASOURCE));
    ASSERT_THAT(data_broker.release(), Eq(nullptr));
}

class FolderDataBrokerTests : public Test {
 public:
    FolderDataBroker* data_broker;
    void SetUp() override {
        data_broker = new FolderDataBroker("/path/to/file");
        data_broker->set_io_(nullptr);
    }
    void TearDown() override {
        delete data_broker;
        data_broker = nullptr;
    }
};

TEST_F(FolderDataBrokerTests, ConnectPreparesFileList) {
    auto return_code = data_broker->Connect();
    ASSERT_THAT(return_code, Eq(WorkerErrorCode::ERR__NO_ERROR));
}

}
