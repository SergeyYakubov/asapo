#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "asapo/unittests/MockFabric.h"
#include <asapo/common/networking.h>
#include "../src/fabric_consumer_client.h"
#include "../../../../common/cpp/src/system_io/system_io.h"

using namespace asapo;

using ::testing::Test;
using ::testing::Ne;
using ::testing::Eq;
using ::testing::_;
using ::testing::SetArgPointee;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::StrictMock;
using ::testing::Expectation;

TEST(FabricConsumerClient, Constructor) {
    FabricConsumerClient client;
    ASSERT_THAT(dynamic_cast<fabric::FabricFactory*>(client.factory__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<fabric::FabricClient*>(client.client__.get()), Eq(nullptr));
}

MATCHER_P6(M_CheckSendRequest, op_code, buf_id, data_size, mr_addr, mr_length, mr_key,
           "Checks if a valid GenericRequestHeader was Send") {
    auto data = (GenericRequestHeader*) arg;
    auto mr = (fabric::MemoryRegionDetails*) &data->message;
    return data->op_code == op_code
           && data->data_id == uint64_t(buf_id)
           && data->data_size == uint64_t(data_size)
           && mr->addr == uint64_t(mr_addr)
           && strcmp(data->api_version, "v0.1") == 0
           && mr->length == uint64_t(mr_length)
           && mr->key == uint64_t(mr_key);
}

ACTION_P(A_WriteSendResponse, error_code) {
    ((asapo::SendResponse*)arg2)->op_code = asapo::kOpcodeGetBufferData;
    ((asapo::SendResponse*)arg2)->error_code = error_code;
}

class FabricConsumerClientTests : public Test {
  public:
    FabricConsumerClient client;
    StrictMock<fabric::MockFabricFactory> mock_fabric_factory;
    StrictMock<fabric::MockFabricClient> mock_fabric_client;

    void SetUp() override {
        client.factory__ = std::unique_ptr<fabric::FabricFactory> {&mock_fabric_factory};
    }
    void TearDown() override {
        client.factory__.release();
        client.client__.release();
    }

  public:
    void ExpectInit(bool ok);
    void ExpectAddedConnection(const std::string& address, bool ok, fabric::FabricAddress result);
    void ExpectTransfer(void** outputData, fabric::FabricAddress serverAddr,
                        fabric::FabricMessageId messageId, bool sendOk, bool recvOk,
                        NetworkErrorCode serverResponse);
};

void FabricConsumerClientTests::ExpectInit(bool ok) {
    EXPECT_CALL(mock_fabric_factory, CreateClient_t(_/*err*/))
    .WillOnce(DoAll(
                  SetArgPointee<0>(ok ? nullptr : fabric::FabricErrorTemplates::kInternalError.Generate().release()),
                  Return(&mock_fabric_client)
              ));
}

void FabricConsumerClientTests::ExpectAddedConnection(const std::string& address, bool ok,
        fabric::FabricAddress result) {
    EXPECT_CALL(mock_fabric_client, AddServerAddress_t(address, _/*err*/))
    .WillOnce(DoAll(
                  SetArgPointee<1>(ok ? nullptr : fabric::FabricErrorTemplates::kInternalError.Generate().release()),
                  Return(result)
              ));
}

void FabricConsumerClientTests::ExpectTransfer(void** outputData, fabric::FabricAddress serverAddr,
                                               fabric::FabricMessageId messageId, bool sendOk, bool recvOk,
                                               NetworkErrorCode serverResponse) {
    static fabric::MemoryRegionDetails mrDetails{};
    mrDetails.addr = 0x124;
    mrDetails.length = 4123;
    mrDetails.key = 20;

    auto mr = new StrictMock<fabric::MockFabricMemoryRegion>();
    EXPECT_CALL(mock_fabric_client, ShareMemoryRegion_t(_, 4123, _/*err*/)).WillOnce(DoAll(
                SaveArg<0>(outputData),
                Return(mr)
            ));
    Expectation getDetailsCall = EXPECT_CALL(*mr, GetDetails()).WillOnce(Return(&mrDetails));


    Expectation sendCall = EXPECT_CALL(mock_fabric_client, Send_t(serverAddr, messageId,
                                       M_CheckSendRequest(kOpcodeGetBufferData, 78954, 4123, 0x124, 4123, 20),
                                       sizeof(GenericRequestHeader), _)).After(getDetailsCall)
                           .WillOnce(SetArgPointee<4>(sendOk ? nullptr : fabric::FabricErrorTemplates::kInternalError.Generate().release()));

    if (sendOk) {
        Expectation recvCall = EXPECT_CALL(mock_fabric_client, Recv_t(serverAddr, messageId, _,
                                           sizeof(GenericNetworkResponse), _))
                               .After(sendCall)
                               .WillOnce(DoAll(
                                             SetArgPointee<4>(recvOk ? nullptr : fabric::FabricErrorTemplates::kInternalError.Generate().release()),
                                             A_WriteSendResponse(serverResponse)
                                         ));
        EXPECT_CALL(*mr, Destructor()).After(recvCall);
    } else {
        EXPECT_CALL(*mr, Destructor()).After(sendCall);
    }

}

TEST_F(FabricConsumerClientTests, GetData_Error_Init) {
    ExpectInit(false);

    MessageData expectedMessageData;
    MessageMeta expectedInfo{};
    expectedInfo.source = "host:1234";
    Error err = client.GetData(&expectedInfo, &expectedMessageData);

    ASSERT_THAT(err, Eq(fabric::FabricErrorTemplates::kInternalError));
}

TEST_F(FabricConsumerClientTests, GetData_Error_AddConnection) {
    ExpectInit(true);
    ExpectAddedConnection("host:1234", false, -1);

    MessageData expectedMessageData;
    MessageMeta expectedInfo{};
    expectedInfo.source = "host:1234";
    Error err = client.GetData(&expectedInfo, &expectedMessageData);
    ASSERT_THAT(err, Eq(fabric::FabricErrorTemplates::kInternalError));

    // Make sure that the connection was not saved
    ExpectAddedConnection("host:1234", false, -1);
    err = client.GetData(&expectedInfo, &expectedMessageData);

    ASSERT_THAT(err, Eq(fabric::FabricErrorTemplates::kInternalError));
}

TEST_F(FabricConsumerClientTests, GetData_ShareMemoryRegion_Error) {
    ExpectInit(true);
    ExpectAddedConnection("host:1234", true, 0);

    MessageData expectedMessageData;
    MessageMeta expectedInfo{};
    expectedInfo.source = "host:1234";
    expectedInfo.size = 4123;

    EXPECT_CALL(mock_fabric_client, ShareMemoryRegion_t(_, 4123, _/*err*/))
    .WillOnce(DoAll(
                  SetArgPointee<2>(fabric::FabricErrorTemplates::kInternalError.Generate().release()),
                  Return(nullptr)
              ));

    Error err = client.GetData(&expectedInfo, &expectedMessageData);

    ASSERT_THAT(err, Eq(fabric::FabricErrorTemplates::kInternalError));
}

TEST_F(FabricConsumerClientTests, GetData_SendFailed) {
    ExpectInit(true);
    ExpectAddedConnection("host:1234", true, 0);

    MessageData expectedMessageData;
    MessageMeta expectedInfo{};
    expectedInfo.source = "host:1234";
    expectedInfo.size = 4123;
    expectedInfo.buf_id = 78954;

    void* outData = nullptr;
    ExpectTransfer(&outData, 0, 0, false, false, kNetErrorNoError);

    Error err = client.GetData(&expectedInfo, &expectedMessageData);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(expectedMessageData.get(), Eq(nullptr));
}

TEST_F(FabricConsumerClientTests, GetData_RecvFailed) {
    ExpectInit(true);
    ExpectAddedConnection("host:1234", true, 0);

    MessageData expectedMessageData;
    MessageMeta expectedInfo{};
    expectedInfo.source = "host:1234";
    expectedInfo.size = 4123;
    expectedInfo.buf_id = 78954;

    void* outData = nullptr;
    ExpectTransfer(&outData, 0, 0, true, false, kNetErrorNoError);

    Error err = client.GetData(&expectedInfo, &expectedMessageData);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(expectedMessageData.get(), Eq(nullptr));
}

TEST_F(FabricConsumerClientTests, GetData_ServerError) {
    ExpectInit(true);
    ExpectAddedConnection("host:1234", true, 0);

    MessageData expectedMessageData;
    MessageMeta expectedInfo{};
    expectedInfo.source = "host:1234";
    expectedInfo.size = 4123;
    expectedInfo.buf_id = 78954;

    void* outData = nullptr;
    ExpectTransfer(&outData, 0, 0, true, true, kNetErrorInternalServerError);

    Error err = client.GetData(&expectedInfo, &expectedMessageData);

    ASSERT_THAT(err, Ne(nullptr));
    ASSERT_THAT(expectedMessageData.get(), Eq(nullptr));
}

TEST_F(FabricConsumerClientTests, GetData_Ok) {
    ExpectInit(true);
    ExpectAddedConnection("host:1234", true, 0);

    MessageData expectedMessageData;
    MessageMeta expectedInfo{};
    expectedInfo.source = "host:1234";
    expectedInfo.size = 4123;
    expectedInfo.buf_id = 78954;

    void* outData = nullptr;
    ExpectTransfer(&outData, 0, 0, true, true, kNetErrorNoError);

    Error err = client.GetData(&expectedInfo, &expectedMessageData);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(expectedMessageData.get(), Eq(outData));
}

TEST_F(FabricConsumerClientTests, GetData_Ok_UsedCahedConnection) {
    ExpectInit(true);
    ExpectAddedConnection("host:1234", true, 0);

    MessageData expectedMessageData;
    MessageMeta expectedInfo{};
    expectedInfo.source = "host:1234";
    expectedInfo.size = 4123;
    expectedInfo.buf_id = 78954;

    void* outData = nullptr;
    ExpectTransfer(&outData, 0, 0, true, true, kNetErrorNoError);

    Error err = client.GetData(&expectedInfo, &expectedMessageData);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(expectedMessageData.get(), Eq(outData));

    outData = nullptr;
    ExpectTransfer(&outData, 0, 1, true, true, kNetErrorNoError);

    err = client.GetData(&expectedInfo, &expectedMessageData);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(expectedMessageData.get(), Eq(outData));
}

TEST_F(FabricConsumerClientTests, GetData_Ok_SecondConnection) {
    ExpectInit(true);
    ExpectAddedConnection("host:1234", true, 0);

    MessageData expectedMessageData;
    MessageMeta expectedInfo{};
    expectedInfo.source = "host:1234";
    expectedInfo.size = 4123;
    expectedInfo.buf_id = 78954;

    void* outData = nullptr;
    ExpectTransfer(&outData, 0, 0, true, true, kNetErrorNoError);

    Error err = client.GetData(&expectedInfo, &expectedMessageData);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(expectedMessageData.get(), Eq(outData));

    ExpectAddedConnection("host:1235", true, 54);
    expectedInfo.source = "host:1235";

    outData = nullptr;
    ExpectTransfer(&outData, 54, 1, true, true, kNetErrorNoError);

    err = client.GetData(&expectedInfo, &expectedMessageData);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(expectedMessageData.get(), Eq(outData));
}
