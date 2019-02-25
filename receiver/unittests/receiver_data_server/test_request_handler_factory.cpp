#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "unittests/MockLogger.h"
#include "../../src/receiver_data_server/receiver_data_server.h"
#include "../../src/receiver_data_server/receiver_data_server_request_handler_factory.h"
#include "../../src/receiver_data_server/receiver_data_server_request_handler.h"



using ::testing::Test;
using ::testing::Gt;
using ::testing::Ge;
using ::testing::Le;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Ref;
using ::testing::Return;
using ::testing::_;
using ::testing::SetArgPointee;
using ::testing::NiceMock;
using ::testing::HasSubstr;


using asapo::ReceiverDataServer;
using asapo::ReceiverDataServerRequestHandlerFactory;


namespace {

TEST(ReceiverDataServerRequestHandlerFactory, Constructor) {
    ReceiverDataServer data_server{"", asapo::LogLevel::Debug, 4, nullptr};
    ReceiverDataServerRequestHandlerFactory factory((asapo::NetServer*)&data_server, nullptr);
    auto handler = factory.NewRequestHandler(1, nullptr);
    ASSERT_THAT(dynamic_cast<const asapo::ReceiverDataServerRequestHandler*>(handler.get()), Ne(nullptr));
}


}
