#include <gmock/gmock.h>

#include "consumer/data_broker.h"
#include "../src/server_data_broker.h"
#include "common/error.h"

using asapo::DataBrokerFactory;
using asapo::DataBroker;
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


TEST_F(DataBrokerFactoryTests, CreateServerDataSource_tcp) {

    auto data_broker = DataBrokerFactory::CreateServerBroker("server", "path", false, asapo::SourceCredentials{"beamtime_id", "", "", "token"},
                       "tcp", &error);

    ASSERT_THAT(error, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<ServerDataBroker*>(data_broker.get()), Ne(nullptr));
}

TEST_F(DataBrokerFactoryTests, CreateServerDataSource_fabric) {

    auto data_broker = DataBrokerFactory::CreateServerBroker("server", "path", false, asapo::SourceCredentials{"beamtime_id", "", "", "token"},
                       "fabric", &error);

    ASSERT_THAT(error, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<ServerDataBroker*>(data_broker.get()), Ne(nullptr));
}

TEST_F(DataBrokerFactoryTests, CreateServerDataSource_invalid) {

    auto data_broker = DataBrokerFactory::CreateServerBroker("server", "path", false, asapo::SourceCredentials{"beamtime_id", "", "", "token"},
                       "test", &error);

    ASSERT_THAT(error, Ne(nullptr));
    ASSERT_THAT(dynamic_cast<ServerDataBroker*>(data_broker.get()), Eq(nullptr));
}


}
