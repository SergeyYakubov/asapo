#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "unittests/MockIO.h"
#include "unittests/MockDatabase.h"
#include "../src/connection.h"
#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_factory.h"
#include "../src/request_handler.h"
#include "../src/request_handler_file_write.h"
#include "../src/request_handler_db_write.h"
#include "../src/request_handler_authorize.h"

#include "database/database.h"

#include "receiver_mocking.h"
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
using ::asapo::GenericRequestHeader;
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
    RequestFactory factory{nullptr};
    Error err{nullptr};
    GenericRequestHeader generic_request_header;
    ReceiverConfig config;
    std::string origin_uri{"origin_uri"};
    void SetUp() override {
        generic_request_header.op_code = asapo::Opcode::kOpcodeTransferData;
        generic_request_header.custom_data[asapo::kPosIngestMode] = asapo::kDefaultIngestMode;
        config.write_to_disk = true;
        config.write_to_db = true;
        SetReceiverConfig(config, "none");
    }
    void TearDown() override {
    }
};

TEST_F(FactoryTests, ErrorOnWrongCode) {
    generic_request_header.op_code = asapo::Opcode::kOpcodeUnknownOp;
    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);

    ASSERT_THAT(err, Ne(nullptr));
}

TEST_F(FactoryTests, ReturnsDataRequestOnkNetOpcodeSendDataCode) {
    for (auto code : std::vector<asapo::Opcode> {asapo::Opcode::kOpcodeTransferData, asapo::Opcode::kOpcodeTransferSubsetData}) {
        generic_request_header.op_code = code;
        auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);

        ASSERT_THAT(err, Eq(nullptr));
        ASSERT_THAT(dynamic_cast<asapo::Request*>(request.get()), Ne(nullptr));
        ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
        ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerFileWrite*>(request->GetListHandlers()[1]), Ne(nullptr));
        ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbWrite*>(request->GetListHandlers().back()), Ne(nullptr));
    }
}


TEST_F(FactoryTests, ReturnsDataRequestForAuthorizationCode) {
    generic_request_header.op_code = asapo::Opcode::kOpcodeAuthorize;
    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::Request*>(request.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
}


TEST_F(FactoryTests, DoNotAddDiskWriterIfNotWantedInConfig) {
    config.write_to_disk = false;

    SetReceiverConfig(config, "none");

    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(request->GetListHandlers().size(), Eq(2));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbWrite*>(request->GetListHandlers().back()), Ne(nullptr));
}

TEST_F(FactoryTests, DoNotAddDiskWriterIfNotWantedInRequest) {
    generic_request_header.custom_data[asapo::kPosIngestMode] = asapo::IngestModeFlags::kTransferData;
    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(request->GetListHandlers().size(), Eq(2));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbWrite*>(request->GetListHandlers().back()), Ne(nullptr));
}

TEST_F(FactoryTests, DoNotAddDbWriterIfNotWanted) {
    config.write_to_db = false;

    SetReceiverConfig(config, "none");

    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(request->GetListHandlers().size(), Eq(2));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerFileWrite*>(request->GetListHandlers()[1]), Ne(nullptr));
}

TEST_F(FactoryTests, CachePassedToRequest) {
    RequestFactory factory{std::shared_ptr<asapo::DataCache>{new asapo::DataCache{0, 0}}};

    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(request->cache__, Ne(nullptr));

}

TEST_F(FactoryTests, ReturnsMetaDataRequestOnkOpcodeTransferMetaData) {
    generic_request_header.op_code = asapo::Opcode::kOpcodeTransferMetaData;
    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::Request*>(request.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::Request*>(request->cache__), Eq(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbMetaWrite*>(request->GetListHandlers().back()), Ne(nullptr));
}


TEST_F(FactoryTests, DonNotGenerateMetadataRequestIfNoDbConfigured) {
    config.write_to_db = false;

    SetReceiverConfig(config, "none");


    generic_request_header.op_code = asapo::Opcode::kOpcodeTransferMetaData;
    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kReject));
}


}
