#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "asapo/unittests/MockLogger.h"
#include "../../../src/receiver_data_server/receiver_data_server.h"
#include "../../../src/receiver_data_server/request_handler/receiver_data_server_request_handler.h"

#include "../../receiver_mocking.h"
#include "../receiver_dataserver_mocking.h"
#include "../../../src/receiver_data_server/receiver_data_server_error.h"
#include "asapo/common/io_error.h"

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
using ::testing::DoAll;

using asapo::ReceiverDataServer;
using asapo::ReceiverDataServerRequestHandler;

namespace {

MATCHER_P3(M_CheckResponse, op_code, error_code, message,
           "Checks if a valid GenericNetworkResponse was used") {
    return ((asapo::GenericNetworkResponse*)arg)->op_code == op_code
           && ((asapo::GenericNetworkResponse*)arg)->error_code == uint64_t(error_code);
}

TEST(RequestHandlerTest, Constructor) {
    asapo::Statistics stat;
    ReceiverDataServerRequestHandler handler{nullptr, nullptr, &stat};
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(handler.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::Statistics*>(handler.statistics__), Ne(nullptr));
}


class RequestHandlerTests : public Test {
  public:
    asapo::MockNetServer mock_net;
    asapo::MockDataCache mock_cache;
    NiceMock<asapo::MockStatistics> mock_stat;
    ReceiverDataServerRequestHandler handler{&mock_net, &mock_cache, &mock_stat};
    ReceiverDataServerRequestHandler handler_no_cache{&mock_net, nullptr, &mock_stat};
    NiceMock<asapo::MockLogger> mock_logger;
    uint64_t expected_data_size = 1001243214;
    uint64_t expected_meta_size = 100;
    uint64_t expected_buf_id = 12345;
    uint64_t expected_source_id = 11;
    asapo::CacheMeta expected_meta;
    bool retry;
    asapo::GenericRequestHeader header{asapo::kOpcodeGetBufferData, expected_buf_id, expected_data_size,
              expected_meta_size, ""};
    asapo::ReceiverDataServerRequest request{std::move(header), expected_source_id};
    uint8_t tmp;
    void SetUp() override {
        handler.log__ = &mock_logger;
    }
    void TearDown() override {
    }
    void MockGetSlotAndUnlockIt(bool return_without_error = true);
    void MockSendResponse(asapo::NetworkErrorCode expected_response_code, bool return_without_error);
    void MockSendResponseAndSlotData(asapo::NetworkErrorCode expected_response_code, bool return_without_error);


};

void RequestHandlerTests::MockGetSlotAndUnlockIt(bool return_without_error) {
    EXPECT_CALL(mock_cache, GetSlotToReadAndLock(expected_buf_id, expected_data_size, _)).WillOnce(DoAll(
                SetArgPointee<2>(return_without_error ? &expected_meta : nullptr),
                Return(return_without_error ? &tmp : nullptr)
            ));
    if (return_without_error) {
        EXPECT_CALL(mock_cache, UnlockSlot(_));
    }
}

void RequestHandlerTests::MockSendResponse(asapo::NetworkErrorCode expected_response_code, bool return_without_error) {
    EXPECT_CALL(mock_net, SendResponse_t(
                    &request,
                    M_CheckResponse(asapo::kOpcodeGetBufferData, expected_response_code, "")
                )).WillOnce(
                    Return(return_without_error ? nullptr : asapo::IOErrorTemplates::kUnknownIOError.Generate().release())
                );
}

void RequestHandlerTests::MockSendResponseAndSlotData(asapo::NetworkErrorCode expected_response_code,
        bool return_without_error) {
    EXPECT_CALL(mock_net, SendResponseAndSlotData_t(
                    &request,
                    M_CheckResponse(asapo::kOpcodeGetBufferData, expected_response_code, ""),
                    &expected_meta
                )).WillOnce(
                    Return(return_without_error ? nullptr : asapo::IOErrorTemplates::kUnknownIOError.Generate().release())
                );
}

TEST_F(RequestHandlerTests, RequestAlwaysReady) {
    auto res  = handler.ReadyProcessRequest();

    ASSERT_THAT(res, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_WrongOpCode) {
    request.header.op_code = asapo::kOpcodeUnknownOp;
    MockSendResponse(asapo::kNetErrorWrongRequest, false);
    EXPECT_CALL(mock_net, HandleAfterError_t(expected_source_id));

    EXPECT_CALL(mock_logger, Error(HasSubstr("wrong request")));

    auto success = handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_WrongClientVersion) {
    strcpy(request.header.api_version, "v0.2");
    MockSendResponse(asapo::kNetErrorNotSupported, false);
    EXPECT_CALL(mock_net, HandleAfterError_t(expected_source_id));

    EXPECT_CALL(mock_logger, Error(HasSubstr("unsupported client")));

    auto success = handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_ReturnsNoDataWhenCacheNotUsed) {
    MockSendResponse(asapo::kNetErrorNoData, true);

    auto success  = handler_no_cache.ProcessRequestUnlocked(&request, &retry);
    EXPECT_CALL(mock_logger, Debug(_)).Times(0);

    ASSERT_THAT(success, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_ReadSlotReturnsNull) {
    MockGetSlotAndUnlockIt(false);
    MockSendResponse(asapo::kNetErrorNoData, true);
    EXPECT_CALL(mock_logger, Debug(HasSubstr("not found")));

    auto success = handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_ReadSlotErrorSendingResponse) {
    MockGetSlotAndUnlockIt(true);
    MockSendResponseAndSlotData(asapo::kNetErrorNoError, false);
    EXPECT_CALL(mock_net, HandleAfterError_t(_));

    auto success  = handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_Ok) {
    MockGetSlotAndUnlockIt(true);
    MockSendResponseAndSlotData(asapo::kNetErrorNoError, true);
    EXPECT_CALL(mock_stat, IncreaseRequestCounter_t());
    EXPECT_CALL(mock_stat, IncreaseRequestDataVolume_t(expected_data_size));
    auto success  = handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}

}
