#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <asapo/unittests/MockIO.h>
#include "../../src/connection.h"
#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/request_handler/request_handler.h"
#include "../../src/request_handler/request_handler_receive_metadata.h"
#include "asapo/database/database.h"
#include "asapo/unittests/MockLogger.h"

#include "../receiver_mocking.h"
#include "../mock_receiver_config.h"


using namespace testing;
using namespace asapo;


namespace {

TEST(ReceiveData, Constructor) {
    RequestHandlerReceiveMetaData handler;
    ASSERT_THAT(dynamic_cast<asapo::IO*>(handler.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(handler.log__), Ne(nullptr));
}

class ReceiveMetaDataHandlerTests : public Test {
  public:
    GenericRequestHeader generic_request_header;
    asapo::SocketDescriptor socket_fd_{1};
    uint64_t data_size_ {100};
    uint64_t data_id_{15};
    uint64_t expected_slot_id{16};
    std::string expected_origin_uri = "origin_uri";
    std::string expected_metadata = "meta";
    uint64_t expected_metadata_size = expected_metadata.size();
    asapo::Opcode expected_op_code = asapo::kOpcodeTransferData;
    std::unique_ptr<Request> request;
    NiceMock<MockIO> mock_io;
    RequestHandlerReceiveMetaData handler;
    NiceMock<asapo::MockLogger> mock_logger;
    StrictMock<asapo::MockInstancedStatistics>* mock_instanced_statistics;

    void SetUp() override {
        generic_request_header.data_size = data_size_;
        generic_request_header.data_id = data_id_;
        generic_request_header.meta_size = expected_metadata_size;
        generic_request_header.op_code = expected_op_code;
        generic_request_header.custom_data[asapo::kPosIngestMode] = asapo::kDefaultIngestMode;
        mock_instanced_statistics = new StrictMock<asapo::MockInstancedStatistics>;
        request.reset(new Request{generic_request_header, socket_fd_, expected_origin_uri, nullptr, nullptr,
                                  std::unique_ptr<asapo::MockInstancedStatistics>{mock_instanced_statistics}});
        handler.io__ = std::unique_ptr<asapo::IO> {&mock_io};
        handler.log__ = &mock_logger;
    }
    void TearDown() override {
        handler.io__.release();
    }
    void ExpectReceive(uint64_t expected_size, bool ok = true);
    void ExpectReceiveMetaData(bool ok = true);
};

ACTION_P(CopyStr, value) {
    if (value.size() <= arg2 && value.size() > 0) {
        memcpy(static_cast<char*>(arg1), value.c_str(), value.size());
    }
}


void ReceiveMetaDataHandlerTests::ExpectReceive(uint64_t expected_size, bool ok) {
    EXPECT_CALL(mock_io, Receive_t(socket_fd_, _, expected_size, _)).WillOnce(
        DoAll(
            CopyStr(expected_metadata),
            SetArgPointee<3>(ok ? nullptr : new asapo::IOError("Test Read Error", "", asapo::IOErrorType::kReadError)),
            Return(ok ? expected_size : 0)
        ));
    EXPECT_CALL(*mock_instanced_statistics, AddIncomingBytes(ok ? expected_size : 0));
}

void ReceiveMetaDataHandlerTests::ExpectReceiveMetaData(bool ok) {
    ExpectReceive(expected_metadata_size, ok);
}

TEST_F(ReceiveMetaDataHandlerTests, CheckStatisticEntity) {
    auto entity = handler.GetStatisticEntity();
    ASSERT_THAT(entity, Eq(asapo::StatisticEntity::kNetworkIncoming));
}

TEST_F(ReceiveMetaDataHandlerTests, HandleReturnsErrorOnMetaDataReceive) {
    ExpectReceiveMetaData(false);
    auto err = handler.ProcessRequest(request.get());
    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kProcessingError));
}

TEST_F(ReceiveMetaDataHandlerTests, HandleReturnsOK) {
    ExpectReceiveMetaData(true);
    auto err = handler.ProcessRequest(request.get());
    ASSERT_THAT(err, Eq(nullptr));
}


}
