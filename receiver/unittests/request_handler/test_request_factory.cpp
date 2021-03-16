#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockDatabase.h"
#include "../../src/connection.h"
#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/request_handler/request_factory.h"
#include "../../src/request_handler/request_handler.h"
#include "../../src/request_handler/request_handler_file_process.h"
#include "../../src/request_handler/request_handler_db_write.h"
#include "../../src/request_handler/request_handler_authorize.h"
#include "../../src/request_handler/request_handler_db_stream_info.h"
#include "../../src/request_handler/request_handler_db_last_stream.h"

#include "../../src/request_handler/request_handler_receive_data.h"
#include "../../src/request_handler/request_handler_receive_metadata.h"

#include "asapo/database/database.h"

#include "../receiver_mocking.h"
#include "../mock_receiver_config.h"


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

TEST_F(FactoryTests, ReturnsDataRequestOnkNetOpcodeSendCode) {
    for (auto code : std::vector<asapo::Opcode> {asapo::Opcode::kOpcodeTransferData, asapo::Opcode::kOpcodeTransferDatasetData}) {
        generic_request_header.op_code = code;
        auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);

        ASSERT_THAT(err, Eq(nullptr));
        ASSERT_THAT(dynamic_cast<asapo::Request*>(request.get()), Ne(nullptr));
        ASSERT_THAT(request->GetListHandlers().size(), Eq(5));
        ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
        ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerReceiveMetaData*>(request->GetListHandlers()[1]), Ne(nullptr));
        ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerReceiveData*>(request->GetListHandlers()[2]), Ne(nullptr));
        ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerFileProcess*>(request->GetListHandlers()[3]), Ne(nullptr));
        ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbWrite*>(request->GetListHandlers().back()), Ne(nullptr));
    }
}

TEST_F(FactoryTests, ReturnsDataRequestOnkNetOpcodeSendCodeLargeFile) {
    for (auto code : std::vector<asapo::Opcode> {asapo::Opcode::kOpcodeTransferData, asapo::Opcode::kOpcodeTransferDatasetData}) {
        generic_request_header.op_code = code;
        config.receive_to_disk_threshold_mb = 0;
        SetReceiverConfig(config, "none");

        generic_request_header.data_size = 1;
        auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);

        ASSERT_THAT(err, Eq(nullptr));
        ASSERT_THAT(dynamic_cast<asapo::Request*>(request.get()), Ne(nullptr));
        ASSERT_THAT(request->GetListHandlers().size(), Eq(4));
        ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
        ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerReceiveMetaData*>(request->GetListHandlers()[1]), Ne(nullptr));
        ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerFileProcess*>(request->GetListHandlers()[2]), Ne(nullptr));
        ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbWrite*>(request->GetListHandlers().back()), Ne(nullptr));
    }
}


TEST_F(FactoryTests, ReturnsDataRequestForAuthorizationCode) {
    generic_request_header.op_code = asapo::Opcode::kOpcodeAuthorize;
    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::Request*>(request.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerReceiveMetaData*>(request->GetListHandlers()[0]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[1]), Ne(nullptr));
}

TEST_F(FactoryTests, DoNotAddDiskAndDbWriterIfNotWantedInRequest) {
    generic_request_header.custom_data[asapo::kPosIngestMode] = asapo::IngestModeFlags::kTransferData;
    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(request->GetListHandlers().size(), Eq(3));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
}

TEST_F(FactoryTests, DoNotAddDbWriterIfNotWanted) {
    generic_request_header.custom_data[asapo::kPosIngestMode] = asapo::IngestModeFlags::kTransferData | asapo::IngestModeFlags::kStoreInFilesystem;

    SetReceiverConfig(config, "none");

    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(request->GetListHandlers().size(), Eq(4));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerReceiveMetaData*>(request->GetListHandlers()[1]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerReceiveData*>(request->GetListHandlers()[2]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerFileProcess*>(request->GetListHandlers()[3]), Ne(nullptr));
}

TEST_F(FactoryTests, CachePassedToRequest) {
    RequestFactory factory{std::shared_ptr<asapo::DataCache>{new asapo::DataCache{0, 0}}};

    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(request->cache__, Ne(nullptr));

}

TEST_F(FactoryTests, ReturnsMetaDataRequestOnTransferMetaDataOnly) {
    generic_request_header.custom_data[asapo::kPosIngestMode] = asapo::IngestModeFlags::kTransferMetaDataOnly;
    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::Request*>(request.get()), Ne(nullptr));
    ASSERT_THAT(request->GetListHandlers().size(), Eq(4));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerReceiveMetaData*>(request->GetListHandlers()[1]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerReceiveData*>(request->GetListHandlers()[2]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbWrite*>(request->GetListHandlers()[3]), Ne(nullptr));
}

TEST_F(FactoryTests, ReturnsMetaDataRequestOnkOpcodeTransferMetaData) {
    generic_request_header.custom_data[asapo::kPosIngestMode] = asapo::IngestModeFlags::kTransferData | asapo::IngestModeFlags::kStoreInDatabase;
    generic_request_header.op_code = asapo::Opcode::kOpcodeTransferMetaData;
    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::Request*>(request.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::Request*>(request->cache__), Eq(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerReceiveData*>(request->GetListHandlers()[1]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbMetaWrite*>(request->GetListHandlers().back()), Ne(nullptr));
}

TEST_F(FactoryTests, DonNotGenerateRequestIfIngestModeIsWrong) {
    config.receive_to_disk_threshold_mb = 0;

    SetReceiverConfig(config, "none");

    generic_request_header.custom_data[asapo::kPosIngestMode] = asapo::kTransferData;
    generic_request_header.data_size = 1;

    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kBadRequest));
}

TEST_F(FactoryTests, StreamInfoRequest) {
    generic_request_header.op_code = asapo::Opcode::kOpcodeStreamInfo;
    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(request->GetListHandlers().size(), Eq(2));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbStreamInfo*>(request->GetListHandlers()[1]), Ne(nullptr));
}

TEST_F(FactoryTests, LastStreamRequest) {
    generic_request_header.op_code = asapo::Opcode::kOpcodeLastStream;
    auto request = factory.GenerateRequest(generic_request_header, 1, origin_uri, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(request->GetListHandlers().size(), Eq(2));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerAuthorize*>(request->GetListHandlers()[0]), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::RequestHandlerDbLastStream*>(request->GetListHandlers()[1]), Ne(nullptr));
}

}
