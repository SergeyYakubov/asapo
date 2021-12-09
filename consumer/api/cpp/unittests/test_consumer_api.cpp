#include <gmock/gmock.h>

#include "asapo/consumer/consumer.h"
#include "../src/consumer_impl.h"
#include "asapo/common/error.h"

using asapo::ConsumerFactory;
using asapo::Consumer;
using asapo::ConsumerImpl;

using asapo::Error;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;


namespace {

class ConsumerFactoryTests : public Test {
  public:
    Error error;
    void SetUp() override {
        error = asapo::GeneralErrorTemplates::kSimpleError.Generate("SomeErrorToBeOverwritten");
    }
};


TEST_F(ConsumerFactoryTests, CreateServerDataSource) {

    auto consumer = ConsumerFactory::CreateConsumer("server",
                                                    "path",
                                                    false,
                                                    asapo::SourceCredentials{asapo::SourceType::kProcessed,
                                                           "instance", "step", "beamtime_id", "", "", "token"},
                                                    &error);

    ASSERT_THAT(error, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<ConsumerImpl*>(consumer.get()), Ne(nullptr));
}


}
