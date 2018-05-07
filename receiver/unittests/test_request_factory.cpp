#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockDatabase.h"
#include "../src/connection.h"
#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_handler.h"
#include "../src/request_handler_file_write.h"
#include "../src/request_handler_db_write.h"
#include "database/database.h"

#include "mock_statistics.h"
#include "mock_receiver_config.h"


using ::testing::Test;
using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::InSequence;
using ::testing::SetArgPointee;
using ::asapo::Error;
using ::asapo::ErrorInterface;
using ::asapo::GenericNetworkRequestHeader;
using ::asapo::GenericNetworkResponse;
using ::asapo::Opcode;
using ::asapo::Connection;
using ::asapo::MockIO;
using asapo::Request;
using asapo::MockStatistics;

using asapo::StatisticEntity;

using asapo::ReceiverConfig;
using asapo::SetReceiverConfig;
using asapo::RequestFactory;

namespace {


class FactoryTests : public Test {
  public:
    RequestFactory factory;
    Error err{nullptr};
    GenericNetworkRequestHeader generic_request_header;
    ReceiverConfig config;

    void SetUp() override {
        generic_request_header.op_code = asapo::Opcode::kNetOpcodeSendData;
        config.write_to_disk = true;
        config.write_to_db = true;
        SetReceiverConfig(config);
    }
    void TearDown() override {
    }
};

TEST_F(FactoryTests, ErrorOnWrongCode) {
    generic_request_header.op_code = asapo::Opcode::kNetOpcodeUnknownOp;
    auto request = factory.GenerateRequest(generic_request_header, 1, &err);

    ASSERT_THAT(err, Ne(nullptr));
}

TEST_F(FactoryTests, ReturnsDataRequestOnkNetOpcodeSendDataCode) {
    generic_request_header.op_code = asapo::Opcode::kNetOpcodeSendData;
    auto request = factory.GenerateRequest(generic_request_header, 1, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::Request*>(request.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerFileWrite*>(request->GetListHandlers()[0]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbWrite*>(request->GetListHandlers().back()), Ne(nullptr));
}

TEST_F(FactoryTests, DoNotAddDiskWriterIfNotWanted) {
    config.write_to_disk = false;

    SetReceiverConfig(config);

    auto request = factory.GenerateRequest(generic_request_header, 1, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(request->GetListHandlers().size(), Eq(1));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbWrite*>(request->GetListHandlers().back()), Ne(nullptr));
}

TEST_F(FactoryTests, DoNotAddDbWriterIfNotWanted) {
    config.write_to_db = false;

    SetReceiverConfig(config);

    auto request = factory.GenerateRequest(generic_request_header, 1, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(request->GetListHandlers().size(), Eq(1));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerFileWrite*>(request->GetListHandlers()[0]), Ne(nullptr));
}


}
