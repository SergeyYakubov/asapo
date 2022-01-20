#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <asapo/unittests/MockIO.h>
#include "../../src/connection.h"
#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/request_handler/request_handler.h"
#include "../../src/request_handler/request_handler_receive_data.h"
#include "asapo/database/database.h"
#include "asapo/unittests/MockLogger.h"

#include "../receiver_mocking.h"
#include "../mock_receiver_config.h"

using namespace testing;
using namespace asapo;

namespace {

TEST(ReceiveData, Constructor) {
    RequestHandlerReceiveData handler;
    ASSERT_THAT(dynamic_cast<asapo::IO*>(handler.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(handler.log__), Ne(nullptr));
}

class ReceiveDataHandlerTests : public Test {
  public:
    GenericRequestHeader generic_request_header;
    asapo::SocketDescriptor socket_fd_{1};
    uint64_t data_size_ {100};
    uint64_t data_id_{15};
    uint64_t expected_slot_id{16};
    std::string expected_origin_uri = "origin_uri";
    std::string expected_metadata = "meta";
    asapo::Opcode expected_op_code = asapo::kOpcodeTransferData;
    char expected_request_message[asapo::kMaxMessageSize] = "test_message";
    std::unique_ptr<Request> request;
    NiceMock<MockIO> mock_io;
    MockDataCache mock_cache;
    RequestHandlerReceiveData handler;
    NiceMock<asapo::MockLogger> mock_logger;


    void SetUp() override {
        generic_request_header.data_size = data_size_;
        generic_request_header.data_id = data_id_;
        generic_request_header.op_code = expected_op_code;
        generic_request_header.custom_data[asapo::kPosIngestMode] = asapo::kDefaultIngestMode;
        strcpy(generic_request_header.message, expected_request_message);
        request.reset(new Request{generic_request_header, socket_fd_, expected_origin_uri, nullptr, nullptr});
        handler.io__ = std::unique_ptr<asapo::IO> {&mock_io};
        handler.log__ = &mock_logger;
    }
    void TearDown() override {
        handler.io__.release();
    }
    void ExpectReceive(uint64_t expected_size, bool ok = true);
    void ExpectReceiveData(bool ok = true);
};

ACTION_P(CopyStr, value) {
    if (value.size() <= arg2 && value.size() > 0) {
        memcpy(static_cast<char*>(arg1), value.c_str(), value.size());
    }
}


void ReceiveDataHandlerTests::ExpectReceive(uint64_t expected_size, bool ok) {
    EXPECT_CALL(mock_io, Receive_t(socket_fd_, _, expected_size, _)).WillOnce(
        DoAll(
            CopyStr(expected_metadata),
            SetArgPointee<3>(ok ? nullptr : new asapo::IOError("Test Read Error", "", asapo::IOErrorType::kReadError)),
            Return(0)
        ));

}
void ReceiveDataHandlerTests::ExpectReceiveData(bool ok) {
    ExpectReceive(data_size_, ok);
}

TEST_F(ReceiveDataHandlerTests, CheckStatisticEntity) {
    auto entity = handler.GetStatisticEntity();
    ASSERT_THAT(entity, Eq(asapo::StatisticEntity::kNetwork));
}


TEST_F(ReceiveDataHandlerTests, HandleDoesNotReceiveEmptyData) {
    generic_request_header.data_size = 0;
    request.reset(new Request{generic_request_header, socket_fd_, "", nullptr, nullptr});

    EXPECT_CALL(mock_io, Receive_t(_, _, _, _)).Times(0);

    auto err = handler.ProcessRequest(request.get());

    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(ReceiveDataHandlerTests, HandleDoesNotReceiveDataWhenMetadataOnlyWasSent) {
    generic_request_header.data_size = 10;
    generic_request_header.custom_data[asapo::kPosIngestMode] = asapo::kTransferMetaDataOnly;
    request.reset(new Request{generic_request_header, socket_fd_, "", nullptr, nullptr});

    auto err = handler.ProcessRequest(request.get());

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ReceiveDataHandlerTests, HandleReturnsErrorOnDataReceive) {
    ExpectReceiveData(false);
    auto err = handler.ProcessRequest(request.get());
    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kProcessingError));
}

TEST_F(ReceiveDataHandlerTests, HandleReturnsOK) {
    ExpectReceiveData(true);
    auto err = handler.ProcessRequest(request.get());
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(ReceiveDataHandlerTests, HandleGetsMemoryFromCache) {
    request->cache__ = &mock_cache;
    asapo::CacheMeta meta;
    meta.id = expected_slot_id;
    EXPECT_CALL(mock_cache, GetFreeSlotAndLock_t(data_size_, _, _)).WillOnce(
        DoAll(SetArgPointee<1>(&meta),
              SetArgPointee<2>(nullptr),
              Return(&mock_cache)
             ));

    auto err = handler.ProcessRequest(request.get());

    ASSERT_THAT(request->GetSlotId(), Eq(expected_slot_id));
}


TEST_F(ReceiveDataHandlerTests, ErrorGetMemoryFromCache) {
    request->cache__ = &mock_cache;

    EXPECT_CALL(mock_cache, GetFreeSlotAndLock_t(data_size_, _,_)).WillOnce(
        DoAll(SetArgPointee<2>(asapo::ReceiverErrorTemplates::kProcessingError.Generate().release()),
              Return(nullptr)
        ));

    auto err = handler.ProcessRequest(request.get());

    ASSERT_THAT(request->GetSlotId(), Eq(0));
    ASSERT_THAT(err, Eq(nullptr));
}



}
