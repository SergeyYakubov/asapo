#include <gtest/gtest.h>

#include "worker/data_broker.h"
#include "../src/folder_data_broker.h"

using hidra2::DataBrokerFactory;
using hidra2::DataBroker;
using hidra2::FolderDataBroker;
using hidra2::WorkerErrorCode;

namespace {

TEST(DataBrokerFactoryTests, CreateFolderDataSource) {
    WorkerErrorCode return_code;

    auto data_broker = DataBrokerFactory::Create("path/to/file", &return_code);

    EXPECT_EQ(return_code, WorkerErrorCode::ERR__NO_ERROR);
    EXPECT_NE(dynamic_cast<FolderDataBroker*>(data_broker.release()), nullptr);
}

TEST(DataBrokerFactoryTests, FailCreateDataSourceWithEmptySource) {
    WorkerErrorCode return_code;

    auto data_broker = DataBrokerFactory::Create("", &return_code);

    EXPECT_EQ(return_code, WorkerErrorCode::ERR__EMPTY_DATASOURCE);
    EXPECT_EQ(data_broker.release(), nullptr);
}

class FolderDataBrokerTests : public ::testing::Test {
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


TEST_F(FolderDataBrokerTests, ConnectPreparesFileList){
    auto return_code= data_broker->Connect();
    EXPECT_EQ(return_code, WorkerErrorCode::ERR__NO_ERROR);
}


}
