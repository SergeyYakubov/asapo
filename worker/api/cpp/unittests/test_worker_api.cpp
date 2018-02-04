#include <gmock/gmock.h>

#include "worker/data_broker.h"
#include "../src/folder_data_broker.h"
#include "../src/server_data_broker.h"


using hidra2::DataBrokerFactory;
using hidra2::DataBroker;
using hidra2::FolderDataBroker;
using hidra2::ServerDataBroker;

using hidra2::WorkerErrorCode;

using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;



namespace {

TEST(DataBrokerFactoryTests, CreateFolderDataSource) {
    WorkerErrorCode return_code;

    auto data_broker = DataBrokerFactory::CreateFolderBroker("path/to/file", &return_code);

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::kOK));
    ASSERT_THAT(dynamic_cast<FolderDataBroker*>(data_broker.get()), Ne(nullptr));
}

TEST(DataBrokerFactoryTests, FailCreateDataSourceWithEmptySource) {
    WorkerErrorCode return_code;

    auto data_broker = DataBrokerFactory::CreateFolderBroker("", &return_code);

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::kEmptyDatasource));
    ASSERT_THAT(data_broker.release(), Eq(nullptr));
}

TEST(DataBrokerFactoryTests, CreateServerDataSource) {
    WorkerErrorCode return_code;

    auto data_broker = DataBrokerFactory::CreateServerBroker("server", "database", &return_code);

    ASSERT_THAT(return_code, Eq(WorkerErrorCode::kOK));
    ASSERT_THAT(dynamic_cast<ServerDataBroker*>(data_broker.get()), Ne(nullptr));
}


}
