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


TEST_F(DataBrokerFactoryTests, CreateServerDataSource) {

    auto data_broker = DataBrokerFactory::CreateServerBroker("server", "path", false, asapo::SourceCredentials{"beamtime_id", "", "", "token"}, &error);

    ASSERT_THAT(error, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<ServerDataBroker*>(data_broker.get()), Ne(nullptr));
}


}
