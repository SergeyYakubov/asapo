#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "unittests/MockLogger.h"
#include "../../src/receiver_data_server/receiver_data_server.h"
#include "../../src/receiver_data_server/receiver_data_server_request_handler.h"

#include "../receiver_mocking.h"
#include "receiver_dataserver_mocking.h"
#include "../../src/receiver_data_server/receiver_data_server_error.h"
#include "common/io_error.h"

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
using asapo::ReceiverDataServerRequestHandler;


namespace {

MATCHER_P3(M_CheckResponce, op_code, error_code, message,
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
    void MockGetSlot(bool ok = true);
    void MockSendResponce(asapo::NetworkErrorCode err_code, bool ok = true);

};

void RequestHandlerTests::MockGetSlot(bool ok) {
    EXPECT_CALL(mock_cache, GetSlotToReadAndLock(expected_buf_id, expected_data_size, _)).WillOnce(
        Return(ok ? &tmp : nullptr)
    );
}

void RequestHandlerTests::MockSendResponce(asapo::NetworkErrorCode err_code, bool ok) {
    EXPECT_CALL(mock_net, SendData_t(expected_source_id,
                                     M_CheckResponce(asapo::kOpcodeGetBufferData, err_code, ""), sizeof(asapo::GenericNetworkResponse))).WillOnce(
                                         Return(ok ? nullptr : asapo::IOErrorTemplates::kUnknownIOError.Generate().release())
                                     );
}

TEST_F(RequestHandlerTests, RequestAlwaysReady) {
    auto res  = handler.ReadyProcessRequest();

    ASSERT_THAT(res, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequest_WronOpCode) {
    request.header.op_code = asapo::kOpcodeUnknownOp;
    MockSendResponce(asapo::kNetErrorWrongRequest, false);
    EXPECT_CALL(mock_net, HandleAfterError_t(expected_source_id));

    EXPECT_CALL(mock_logger, Error(HasSubstr("wrong request")));

    auto success = handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequestReturnsNoDataWhenCacheNotUsed) {
    MockSendResponce(asapo::kNetErrorNoData, true);

    auto success  = handler_no_cache.ProcessRequestUnlocked(&request, &retry);
    EXPECT_CALL(mock_logger, Debug(_)).Times(0);

    ASSERT_THAT(success, Eq(true));
}

TEST_F(RequestHandlerTests, ProcessRequestReadSlotReturnsNull) {
    MockGetSlot(false);
    MockSendResponce(asapo::kNetErrorNoData, true);
    EXPECT_CALL(mock_logger, Debug(HasSubstr("not found")));

    auto success = handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}


TEST_F(RequestHandlerTests, ProcessRequestReadSlotErrorSendingResponce) {
    MockGetSlot(true);
    MockSendResponce(asapo::kNetErrorNoError, false);
    EXPECT_CALL(mock_net, SendData_t(expected_source_id, &tmp, expected_data_size)).Times(0);
    EXPECT_CALL(mock_cache, UnlockSlot(_));

    auto success  = handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}



TEST_F(RequestHandlerTests, ProcessRequestOk) {
    MockGetSlot(true);
    MockSendResponce(asapo::kNetErrorNoError, true);
    EXPECT_CALL(mock_net, SendData_t(expected_source_id, &tmp, expected_data_size)).WillOnce(
        Return(nullptr)
    );
    EXPECT_CALL(mock_cache, UnlockSlot(_));
    EXPECT_CALL(mock_logger, Debug(HasSubstr("sending")));
    EXPECT_CALL(mock_stat, IncreaseRequestCounter_t());
    EXPECT_CALL(mock_stat, IncreaseRequestDataVolume_t(expected_data_size));
    auto success  = handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
}

}
