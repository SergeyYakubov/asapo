#include <gtest/gtest.h>
#include <unittests/MockIO.h>

#include "../src/request_handler_factory.h"
#include "../src/receiver_discovery_service.h"
#include "../src/request_handler_tcp.h"
#include "mocking.h"

using ::testing::Ne;
using ::testing::Eq;

using asapo:: RequestHandlerFactory;


namespace {

TEST(CreateFactory, Tcp) {
    MockDiscoveryService mock_discovery;
    EXPECT_CALL(mock_discovery, StartCollectingData());

    RequestHandlerFactory factory{asapo::RequestHandlerType::kTcp,&mock_discovery};

    auto handler = factory.NewRequestHandler(1);

    ASSERT_THAT(dynamic_cast<asapo::RequestHandlerTcp*>(handler.get()), Ne(nullptr));

}



}
