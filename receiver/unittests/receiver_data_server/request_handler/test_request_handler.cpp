#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "asapo/unittests/MockLogger.h"
#include "../../../src/receiver_data_server/receiver_data_server.h"
#include "../../../src/receiver_data_server/request_handler/receiver_data_server_request_handler.h"

#include "../../receiver_mocking.h"
#include "../receiver_dataserver_mocking.h"
#include "../../../src/receiver_data_server/receiver_data_server_error.h"
#include "asapo/common/io_error.h"
#include "../../monitoring/receiver_monitoring_mocking.h"

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
using ::testing::StrictMock;
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
    std::unique_ptr<StrictMock<asapo::MockNetServer>> mock_net_ptr;
    std::shared_ptr<StrictMock<asapo::MockReceiverMonitoringClient>> mock_monitoring_ptr;

    std::unique_ptr<ReceiverDataServerRequestHandler> handler_ptr;
    std::unique_ptr<ReceiverDataServerRequestHandler> handler_no_cache_ptr;

    asapo::MockDataCache mock_cache;
    NiceMock<asapo::MockStatistics> mock_stat;
    NiceMock<asapo::MockLogger> mock_logger;
    uint64_t expected_data_size = 1001243214;
    uint64_t expected_meta_size = 100;
    uint64_t expected_buf_id = 12345;
    uint64_t expected_source_id = 11;
    asapo::CacheMeta expected_meta;
    bool retry;
    asapo::GenericRequestHeader header{asapo::kOpcodeGetBufferData, expected_buf_id, expected_data_size,
                                       expected_meta_size, "", "instanceId§pipelineStepId§bt§source§stream"};

    asapo::ReceiverDataServerRequest request{std::move(header), expected_source_id, nullptr};
    uint8_t tmp;
    void SetUp() override {
        mock_net_ptr.reset(new StrictMock<asapo::MockNetServer>);
        mock_monitoring_ptr.reset(new StrictMock<asapo::MockReceiverMonitoringClient>{nullptr});

        ON_CALL(*mock_net_ptr, Monitoring()).WillByDefault(Return(mock_monitoring_ptr));

        handler_ptr.reset(new ReceiverDataServerRequestHandler{mock_net_ptr.get(), &mock_cache, &mock_stat});
        handler_ptr->log__ = &mock_logger;
        handler_no_cache_ptr.reset(new ReceiverDataServerRequestHandler{mock_net_ptr.get(), nullptr, &mock_stat});
        handler_no_cache_ptr->log__ = &mock_logger;
    }
    void TearDown() override {
    }
    void MockGetSlotAndUnlockIt(bool return_without_error = true);
    void MockSendResponse(asapo::NetworkErrorCode expected_response_code, bool return_without_error, bool expect_monitoring);
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

void RequestHandlerTests::MockSendResponse(asapo::NetworkErrorCode expected_response_code, bool return_without_error, bool expect_monitoring) {
    if (expect_monitoring) {
        EXPECT_CALL(*mock_net_ptr, Monitoring());
        EXPECT_CALL(*mock_monitoring_ptr, SendRdsRequestWasMissDataPoint("pipelineStepId", "instanceId", "bt", "source", "stream"));
    }

    EXPECT_CALL(*mock_net_ptr, SendResponse_t(
                    &request,
                    M_CheckResponse(asapo::kOpcodeGetBufferData, expected_response_code, "")
                )).WillOnce(
                    Return(return_without_error ? nullptr : asapo::IOErrorTemplates::kUnknownIOError.Generate().release())
                );
}

void RequestHandlerTests::MockSendResponseAndSlotData(asapo::NetworkErrorCode expected_response_code,
        bool return_without_error) {

    EXPECT_CALL(*mock_net_ptr, Monitoring());
    EXPECT_CALL(*mock_monitoring_ptr, SendReceiverRequestDataPoint("pipelineStepId", "instanceId", "bt", "source", "stream", _, _));

    EXPECT_CALL(*mock_net_ptr, SendResponseAndSlotData_t(
                    &request,
                    M_CheckResponse(asapo::kOpcodeGetBufferData, expected_response_code, ""),
                    &expected_meta
                )).WillOnce(
                    Return(return_without_error ? nullptr : asapo::IOErrorTemplates::kUnknownIOError.Generate().release())
                );
}

TEST_F(RequestHandlerTests, RequestAlwaysReady) {
    auto res  = handler_ptr->ReadyProcessRequest();

    ASSERT_THAT(res, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_WrongOpCode) {
    request.header.op_code = asapo::kOpcodeUnknownOp;
    MockSendResponse(asapo::kNetErrorWrongRequest, false, false);
    EXPECT_CALL(*mock_net_ptr, HandleAfterError_t(expected_source_id));

    EXPECT_CALL(mock_logger, Error(HasSubstr("wrong request")));

    auto success = handler_ptr->ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_WrongClientVersion) {
    strcpy(request.header.api_version, "v0.3");
    MockSendResponse(asapo::kNetErrorNotSupported, false, false);
    EXPECT_CALL(*mock_net_ptr, HandleAfterError_t(expected_source_id));

    EXPECT_CALL(mock_logger, Error(HasSubstr("unsupported client")));

    auto success = handler_ptr->ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_ReturnsNoDataWhenCacheNotUsed) {
    MockSendResponse(asapo::kNetErrorNoData, true, true);

    auto success  = handler_no_cache_ptr->ProcessRequestUnlocked(&request, &retry);
    EXPECT_CALL(mock_logger, Debug(_)).Times(0);

    ASSERT_THAT(success, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_ReadSlotReturnsNull) {
    MockGetSlotAndUnlockIt(false);
    MockSendResponse(asapo::kNetErrorNoData, true, true);
    EXPECT_CALL(mock_logger, Debug(HasSubstr("not found")));

    auto success = handler_ptr->ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_ReadSlotErrorSendingResponse) {
    MockGetSlotAndUnlockIt(true);
    MockSendResponseAndSlotData(asapo::kNetErrorNoError, false);
    EXPECT_CALL(*mock_net_ptr, HandleAfterError_t(_));

    auto success  = handler_ptr->ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_Ok) {
    MockGetSlotAndUnlockIt(true);
    MockSendResponseAndSlotData(asapo::kNetErrorNoError, true);
    EXPECT_CALL(mock_stat, IncreaseRequestCounter_t());
    EXPECT_CALL(mock_stat, IncreaseRequestDataVolume_t(expected_data_size));
    auto success  = handler_ptr->ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}

}
