#include <gmock/gmock.h>

#include "worker/data_broker.h"
#include "../src/folder_data_broker.h"
#include "../src/server_data_broker.h"
#include "common/error.h"

using asapo::DataBrokerFactory;
using asapo::DataBroker;
using asapo::FolderDataBroker;
using asapo::ServerDataBroker;

using asapo::Error;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;


namespace {

class DataBrokerFactoryTests : public Test {
  public:
    Error error;
    void SetUp() override {
        error.reset(new asapo::SimpleError("SomeErrorToBeOverwritten"));
    }
};


TEST_F(DataBrokerFactoryTests, CreateFolderDataSource) {

    auto data_broker = DataBrokerFactory::CreateFolderBroker("path/to/file", &error);

    ASSERT_THAT(error, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<FolderDataBroker*>(data_broker.get()), Ne(nullptr));
}

TEST_F(DataBrokerFactoryTests, FailCreateDataSourceWithEmptySource) {

    auto data_broker = DataBrokerFactory::CreateFolderBroker("", &error);

//    ASSERT_THAT(error->Explain(), Eq(WorkerErrorMessage::kEmptyDatasource));
    ASSERT_THAT(error->Explain(), Eq("Empty Data Source"));
    ASSERT_THAT(data_broker.get(), Eq(nullptr));
}

TEST_F(DataBrokerFactoryTests, CreateServerDataSource) {

    auto data_broker = DataBrokerFactory::CreateServerBroker("server", "database", &error);

    ASSERT_THAT(error, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<ServerDataBroker*>(data_broker.get()), Ne(nullptr));
}


}
