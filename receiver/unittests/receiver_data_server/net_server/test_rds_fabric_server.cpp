#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <asapo/unittests/MockIO.h>
#include <asapo/unittests/MockLogger.h>
#include <asapo/unittests/MockFabric.h>
#include "../../../src/receiver_data_server/net_server/rds_fabric_server.h"
#include "../../../src/receiver_data_server/net_server/fabric_rds_request.h"
#include "../../../../common/cpp/src/system_io/system_io.h"
#include "../../receiver_mocking.h"
#include "../../monitoring/receiver_monitoring_mocking.h"

using ::testing::Ne;
using ::testing::Eq;
using ::testing::Test;
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::DoAll;
using ::testing::SetArgPointee;
using ::testing::Return;
using ::testing::_;

using namespace asapo;

std::string expected_address = "somehost:123";

TEST(RdsFabricServer, Constructor) {
    NiceMock<MockLogger> mock_logger;
    std::shared_ptr<StrictMock<MockReceiverMonitoringClient>> mock_monitoring{new StrictMock<MockReceiverMonitoringClient>};
    RdsFabricServer fabric_server("", &mock_logger, mock_monitoring);
    ASSERT_THAT(dynamic_cast<SystemIO*>(fabric_server.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<fabric::FabricFactory*>(fabric_server.factory__.get()), Ne(nullptr));
    ASSERT_THAT(fabric_server.log__, Eq(&mock_logger));
    ASSERT_THAT(fabric_server.Monitoring(), Eq(mock_monitoring));
}

class RdsFabricServerTests : public Test {
  public:
    NiceMock<MockLogger> mock_logger;
    StrictMock<MockIO> mock_io;
    StrictMock<fabric::MockFabricFactory> mock_fabric_factory;
    StrictMock<fabric::MockFabricServer> mock_fabric_server;
    std::shared_ptr<StrictMock<MockReceiverMonitoringClient>> mock_monitoring;
    std::unique_ptr<RdsFabricServer> rds_server_ptr;

    void SetUp() override {
        mock_monitoring.reset(new StrictMock<MockReceiverMonitoringClient>);
        RdsFabricServer XX{expected_address, &mock_logger, mock_monitoring};

        rds_server_ptr.reset(new RdsFabricServer {expected_address, &mock_logger, mock_monitoring});

        rds_server_ptr->log__ = &mock_logger;
        rds_server_ptr->io__ = std::unique_ptr<IO> {&mock_io};
        rds_server_ptr->factory__ = std::unique_ptr<fabric::FabricFactory> {&mock_fabric_factory};
    }

    void TearDown() override {
        rds_server_ptr->io__.release();
        rds_server_ptr->factory__.release();
        rds_server_ptr->server__.release();
    }

  public:
    void InitServer();
};

void RdsFabricServerTests::InitServer() {
    EXPECT_CALL(mock_io, SplitAddressToHostnameAndPort_t(expected_address)).WillOnce(Return(
                new std::tuple<std::string, uint16_t>("abc", 123)
            ));

    EXPECT_CALL(mock_fabric_factory, CreateAndBindServer_t(_, "abc", 123, _)).WillOnce(DoAll(
                SetArgPointee<3>(fabric::FabricErrorTemplates::kInternalError.Generate().release()),
                Return(&mock_fabric_server)
            ));

    Error err = rds_server_ptr->Initialize();

    ASSERT_THAT(err, Eq(fabric::FabricErrorTemplates::kInternalError));
}

TEST_F(RdsFabricServerTests, Initialize_Ok) {
    InitServer();
}

TEST_F(RdsFabricServerTests, Initialize_Error_CreateAndBindServer) {
    EXPECT_CALL(mock_io, SplitAddressToHostnameAndPort_t(expected_address)).WillOnce(Return(
                new std::tuple<std::string, uint16_t>("abc", 123)
            ));

    EXPECT_CALL(mock_fabric_factory, CreateAndBindServer_t(_, "abc", 123, _)).WillOnce(DoAll(
                SetArgPointee<3>(fabric::FabricErrorTemplates::kInternalError.Generate().release()),
                Return(nullptr)
            ));

    Error err = rds_server_ptr->Initialize();

    ASSERT_THAT(rds_server_ptr->server__, Eq(nullptr));
    ASSERT_THAT(err, Eq(fabric::FabricErrorTemplates::kInternalError));
}

TEST_F(RdsFabricServerTests, Initialize_Error_DoubleInitialize) {
    EXPECT_CALL(mock_io, SplitAddressToHostnameAndPort_t(expected_address)).WillOnce(Return(
                new std::tuple<std::string, uint16_t>("abc", 123)
            ));

    EXPECT_CALL(mock_fabric_factory, CreateAndBindServer_t(_, "abc", 123, _)).WillOnce(Return(
                &mock_fabric_server
            ));

    EXPECT_CALL(mock_fabric_server, GetAddress()).WillOnce(Return(
                "TestAddress"
            ));

    Error err = rds_server_ptr->Initialize();
    ASSERT_THAT(rds_server_ptr->server__, Ne(nullptr));
    ASSERT_THAT(err, Eq(nullptr));

    err = rds_server_ptr->Initialize();
    ASSERT_THAT(rds_server_ptr->server__, Ne(nullptr));
    ASSERT_THAT(err, Ne(nullptr));
}

ACTION_P5(A_WriteToRecvAnyBuffer, op_code, expected_id, remote_mem_addr, remote_mem_length, remote_mem_key) {
    ((GenericRequestHeader*)arg2)->op_code = op_code;
    ((GenericRequestHeader*)arg2)->data_id = static_cast<uint64_t>(expected_id);
    ((fabric::MemoryRegionDetails*) & ((GenericRequestHeader*)arg2)->message)->addr = static_cast<uint64_t>
            (remote_mem_addr);
    ((fabric::MemoryRegionDetails*) & ((GenericRequestHeader*)arg2)->message)->length = static_cast<uint64_t>
            (remote_mem_length);
    ((fabric::MemoryRegionDetails*) & ((GenericRequestHeader*)arg2)->message)->key = static_cast<uint64_t>(remote_mem_key);
}

TEST_F(RdsFabricServerTests, GetNewRequests_Ok) {
    InitServer();

    EXPECT_CALL(mock_fabric_server, RecvAny_t(_/*&src*/, _/*&msgId*/, _/*&buf*/, sizeof(GenericRequestHeader), _/*err*/))
    .WillOnce(DoAll(
                  SetArgPointee<0>(542),
                  SetArgPointee<1>(123),
                  A_WriteToRecvAnyBuffer(asapo::kOpcodeGetBufferData, 30,
                                         90, 10, 23)
              ));

    Error err;
    GenericRequests requests = rds_server_ptr->GetNewRequests(&err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(requests.size(), Eq(1));
    auto req = dynamic_cast<FabricRdsRequest*>(requests[0].get());
    ASSERT_THAT(req->source_id, Eq(542));
    ASSERT_THAT(req->message_id, Eq(123));
    ASSERT_THAT(req->header.op_code, Eq(asapo::kOpcodeGetBufferData));
    ASSERT_THAT(req->header.data_id, Eq(30));
    ASSERT_THAT(req->GetMemoryRegion()->addr, Eq(90));
    ASSERT_THAT(req->GetMemoryRegion()->length, Eq(10));
    ASSERT_THAT(req->GetMemoryRegion()->key, Eq(23));
}

TEST_F(RdsFabricServerTests, GetNewRequests_Error_RecvAny_InternalError) {
    InitServer();

    EXPECT_CALL(mock_fabric_server, RecvAny_t(_/*&src*/, _/*&msgId*/, _/*&buf*/, _/*bufSize*/, _/*err*/))
    .WillOnce(
        SetArgPointee<4>(fabric::FabricErrorTemplates::kInternalError.Generate().release())
    );

    Error err;
    GenericRequests requests = rds_server_ptr->GetNewRequests(&err);

    ASSERT_THAT(err, Eq(fabric::FabricErrorTemplates::kInternalError));
    ASSERT_THAT(requests.size(), Eq(0));
}

TEST_F(RdsFabricServerTests, GetNewRequests_Error_RecvAny_Timeout) {
    InitServer();

    EXPECT_CALL(mock_fabric_server, RecvAny_t(_/*&src*/, _/*&msgId*/, _/*&buf*/, _/*bufSize*/, _/*err*/))
    .WillOnce(
        SetArgPointee<4>(IOErrorTemplates::kTimeout.Generate().release())
    );

    Error err;
    GenericRequests requests = rds_server_ptr->GetNewRequests(&err);

    ASSERT_THAT(err, Eq(IOErrorTemplates::kTimeout));
    ASSERT_THAT(requests.size(), Eq(0));
}

TEST_F(RdsFabricServerTests, SendResponse_Ok) {
    InitServer();

    FabricRdsRequest request(GenericRequestHeader{}, 41, 87, nullptr);
    GenericNetworkResponse response;

    EXPECT_CALL(mock_fabric_server, Send_t(41, 87, &response, sizeof(response), _/*err*/)).Times(1);

    Error err = rds_server_ptr->SendResponse(&request, &response);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RdsFabricServerTests, SendResponse_Error_SendError) {
    InitServer();

    FabricRdsRequest request(GenericRequestHeader{}, 41, 87, nullptr);
    GenericNetworkResponse response;

    EXPECT_CALL(mock_fabric_server, Send_t(41, 87, &response, sizeof(response), _/*err*/)).WillOnce(
        SetArgPointee<4>(fabric::FabricErrorTemplates::kInternalError.Generate().release())
    );

    Error err = rds_server_ptr->SendResponse(&request, &response);

    ASSERT_THAT(err, Eq(fabric::FabricErrorTemplates::kInternalError));
}

TEST_F(RdsFabricServerTests, SendResponseAndSlotData_Ok) {
    InitServer();

    GenericRequestHeader dummyHeader{};
    FabricRdsRequest request(GenericRequestHeader{}, 41, 87, nullptr);
    GenericNetworkResponse response;
    CacheMeta cacheSlot;
    cacheSlot.addr = (void*)0xABC;
    cacheSlot.size = 200;

    EXPECT_CALL(mock_fabric_server, RdmaWrite_t(41, request.GetMemoryRegion(), (void*)0xABC, 200, _/*err*/)).Times(1);
    EXPECT_CALL(mock_fabric_server, Send_t(41, 87, &response, sizeof(response), _/*err*/)).Times(1);

    Error err = rds_server_ptr->SendResponseAndSlotData(&request, &response, &cacheSlot);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RdsFabricServerTests, SendResponseAndSlotData_RdmaWrite_Error) {
    InitServer();

    GenericRequestHeader dummyHeader{};
    FabricRdsRequest request(GenericRequestHeader{}, 41, 87, nullptr);
    GenericNetworkResponse response;
    CacheMeta cacheSlot;
    cacheSlot.addr = (void*)0xABC;
    cacheSlot.size = 200;

    EXPECT_CALL(mock_fabric_server, RdmaWrite_t(41, request.GetMemoryRegion(), (void*)0xABC, 200, _/*err*/)).WillOnce(
        SetArgPointee<4>(fabric::FabricErrorTemplates::kInternalError.Generate().release())
    );

    Error err = rds_server_ptr->SendResponseAndSlotData(&request, &response, &cacheSlot);

    ASSERT_THAT(err, Eq(fabric::FabricErrorTemplates::kInternalError));
}

TEST_F(RdsFabricServerTests, SendResponseAndSlotData_Send_Error) {
    InitServer();

    GenericRequestHeader dummyHeader{};
    FabricRdsRequest request(GenericRequestHeader{}, 41, 87, nullptr);
    GenericNetworkResponse response;
    CacheMeta cacheSlot;
    cacheSlot.addr = (void*)0xABC;
    cacheSlot.size = 200;

    EXPECT_CALL(mock_fabric_server, RdmaWrite_t(41, request.GetMemoryRegion(), (void*)0xABC, 200, _/*err*/)).Times(1);
    EXPECT_CALL(mock_fabric_server, Send_t(41, 87, &response, sizeof(response), _/*err*/)).WillOnce(
        SetArgPointee<4>(fabric::FabricErrorTemplates::kInternalError.Generate().release())
    );

    Error err = rds_server_ptr->SendResponseAndSlotData(&request, &response, &cacheSlot);

    ASSERT_THAT(err, Eq(fabric::FabricErrorTemplates::kInternalError));
}

TEST_F(RdsFabricServerTests, HandleAfterError) {
    InitServer();
    rds_server_ptr->HandleAfterError(2); /* Function does nothing */
}
