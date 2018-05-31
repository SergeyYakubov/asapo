#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"
#include "common/error.h"
#include "io/io.h"

#include "producer/common.h"
#include "producer/producer_error.h"

#include "../src/request_handler_tcp.h"
#include <common/networking.h>
#include "io/io_factory.h"

#include "mocking.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::AllOf;
using testing::NiceMock;

using ::testing::InSequence;
using ::testing::HasSubstr;


TEST(RequestHandlerTcp, Constructor) {
    MockDiscoveryService ds;
    asapo::RequestHandlerTcp request{&ds, 1, nullptr};

    ASSERT_THAT(dynamic_cast<const asapo::IO*>(request.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(request.log__), Ne(nullptr));
    ASSERT_THAT(request.discovery_service__, Eq(&ds));

}

class RequestHandlerTcpTests : public testing::Test {
  public:
    NiceMock<asapo::MockIO> mock_io;
    NiceMock<MockDiscoveryService> mock_discovery_service;

    uint64_t expected_file_id = 42;
    uint64_t expected_file_size = 1337;
    std::string  expected_file_name = "test_name";
    uint64_t expected_thread_id = 2;

    asapo::Opcode expected_op_code = asapo::kOpcodeTransferData;
    void*    expected_file_pointer = (void*)0xC00FE;
    asapo::Error callback_err;
    asapo::GenericRequestHeader header{expected_op_code, expected_file_id, expected_file_size, expected_file_name};
    bool called = false;
    asapo::GenericRequestHeader callback_header;
    asapo::Request request{header, expected_file_pointer, [this](asapo::GenericRequestHeader header, asapo::Error err) {
        called = true;
        callback_err = std::move(err);
        callback_header = header;
    }};

    asapo::Request request_nocallback{header, expected_file_pointer, nullptr};
    testing::NiceMock<asapo::MockLogger> mock_logger;
    uint64_t n_connections{0};
    asapo::RequestHandlerTcp request_handler{&mock_discovery_service, expected_thread_id, &n_connections};

    std::string expected_address1 = {"127.0.0.1:9090"};
    std::string expected_address2 = {"127.0.0.1:9091"};
    asapo::ReceiversList receivers_list{expected_address1, expected_address2};
    asapo::ReceiversList receivers_list2{expected_address2, expected_address1};

    asapo::ReceiversList receivers_list_single{expected_address1};

    std::vector<asapo::SocketDescriptor> expected_sds{83942, 83943};

    void ExpectFailConnect(bool only_once = false);
    void ExpectFailSendHeader(bool only_once = false);
    void ExpectFailSendData(bool only_once = false);
    void ExpectOKConnect(bool only_once = false);
    void ExpectOKSendHeader(bool only_once = false);
    void ExpectOKSendData(bool only_once = false);
    void ExpectFailReceive(bool only_once = false);
    void ExpectOKReceive(bool only_once = true);
    void DoSingleSend(bool connect = true, bool success = true);

    void SetUp() override {
        request_handler.log__ = &mock_logger;
        request_handler.io__.reset(&mock_io);
        ON_CALL(mock_discovery_service, RotatedUriList(_)).
        WillByDefault(Return(receivers_list));

    }
    void TearDown() override {
        request_handler.io__.release();
    }
};

ACTION_P(A_WriteSendDataResponse, error_code) {
    ((asapo::SendDataResponse*)arg1)->op_code = asapo::kOpcodeTransferData;
    ((asapo::SendDataResponse*)arg1)->error_code = error_code;
}

MATCHER_P2(M_CheckSendDataRequest, file_id, file_size,
           "Checks if a valid GenericRequestHeader was Send") {
    return ((asapo::GenericRequestHeader*)arg)->op_code == asapo::kOpcodeTransferData
           && ((asapo::GenericRequestHeader*)arg)->data_id == file_id
           && ((asapo::GenericRequestHeader*)arg)->data_size == file_size;
}


void RequestHandlerTcpTests::ExpectFailConnect(bool only_once) {
    for (auto expected_address : receivers_list) {
        EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_address, _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(asapo::IOErrorTemplates::kInvalidAddressFormat.Generate().release()),
                Return(asapo::kDisconnectedSocketDescriptor)
            ));
        EXPECT_CALL(mock_logger, Debug(AllOf(
                                           HasSubstr("cannot connect"),
                                           HasSubstr(expected_address)
                                       )
                                      ));

        if (only_once) break;
    }

}

void RequestHandlerTcpTests::ExpectFailSendHeader(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, M_CheckSendDataRequest(expected_file_id,
                                    expected_file_size),
                                    sizeof(asapo::GenericRequestHeader), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
                Return(-1)
            ));
        EXPECT_CALL(mock_logger, Debug(AllOf(
                                           HasSubstr("disconnected"),
                                           HasSubstr(receivers_list[i])
                                       )
                                      ));

        EXPECT_CALL(mock_logger, Debug(AllOf(
                                           HasSubstr("cannot send"),
                                           HasSubstr(receivers_list[i])
                                       )
                                      ));
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) break;
        i++;
    }

}

void RequestHandlerTcpTests::ExpectFailSendData(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, expected_file_pointer, expected_file_size, _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
                Return(-1)
            ));
        EXPECT_CALL(mock_logger, Debug(AllOf(
                                           HasSubstr("disconnected"),
                                           HasSubstr(receivers_list[i])
                                       )
                                      ));

        EXPECT_CALL(mock_logger, Debug(AllOf(
                                           HasSubstr("cannot send"),
                                           HasSubstr(receivers_list[i])
                                       )
                                      ));
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) break;
        i++;
    }

}


void RequestHandlerTcpTests::ExpectFailReceive(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Receive_t(expected_sd, _, sizeof(asapo::SendDataResponse), _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
                testing::Return(-1)
            ));
        EXPECT_CALL(mock_logger, Debug(AllOf(
                                           HasSubstr("disconnected"),
                                           HasSubstr(receivers_list[i])
                                       )
                                      ));


        EXPECT_CALL(mock_logger, Debug(AllOf(
                                           HasSubstr("cannot send"),
                                           HasSubstr(receivers_list[i])
                                       )
                                      ));
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) break;
        i++;
    }

}


void RequestHandlerTcpTests::ExpectOKSendData(bool only_once) {
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, expected_file_pointer, expected_file_size, _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                Return(expected_file_size)
            ));
        if (only_once) break;
    }

}



void RequestHandlerTcpTests::ExpectOKSendHeader(bool only_once) {
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, M_CheckSendDataRequest(expected_file_id,
                                    expected_file_size),
                                    sizeof(asapo::GenericRequestHeader), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                Return(sizeof(asapo::GenericRequestHeader))
            ));
        if (only_once) break;
    }

}


void RequestHandlerTcpTests::ExpectOKConnect(bool only_once) {
    int i = 0;
    for (auto expected_address : receivers_list) {
        EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_address, _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(nullptr),
                Return(expected_sds[i])
            ));
        EXPECT_CALL(mock_logger, Info(AllOf(
                                          HasSubstr("connected"),
                                          HasSubstr(expected_address)
                                      )
                                     ));
        if (only_once) break;
        i++;
    }
}


void RequestHandlerTcpTests::ExpectOKReceive(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Receive_t(expected_sd, _, sizeof(asapo::SendDataResponse), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                A_WriteSendDataResponse(asapo::kNetErrorNoError),
                testing::ReturnArg<2>()
            ));
        EXPECT_CALL(mock_logger, Debug(AllOf(
                                           HasSubstr("sent data"),
                                           HasSubstr(receivers_list[i])
                                       )
                                      ));
        if (only_once) break;
        i++;
    }
}

void RequestHandlerTcpTests::DoSingleSend(bool connect, bool success) {
    if (connect) ExpectOKConnect(true);
    ExpectOKSendHeader(true);
    ExpectOKSendData(true);
    if (success) {
        ExpectOKReceive(true);
    } else {
        ExpectFailReceive(true);
    }

    if (connect) {
        EXPECT_CALL(mock_discovery_service, RotatedUriList(_)).
        WillOnce(Return(receivers_list_single));
    }

    request_handler.PrepareProcessingRequestLocked();
    request_handler.ProcessRequestUnlocked(&request);

    Mock::VerifyAndClearExpectations(&mock_io);
    Mock::VerifyAndClearExpectations(&mock_logger);
    Mock::VerifyAndClearExpectations(&mock_discovery_service);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST_F(RequestHandlerTcpTests, CannotProcessRequestIfNotEnoughConnections) {
    EXPECT_CALL(mock_discovery_service, MaxConnections()).WillOnce(Return(0));
    auto res = request_handler.ReadyProcessRequest();
    ASSERT_THAT(res, Eq(false));
}

TEST_F(RequestHandlerTcpTests, CanProcessRequestIfAlreadyConnected) {
    DoSingleSend();
    EXPECT_CALL(mock_discovery_service, MaxConnections()).Times(0);

    auto res = request_handler.ReadyProcessRequest();

    ASSERT_THAT(res, Eq(true));
}

TEST_F(RequestHandlerTcpTests, GetsUriListINotConnected) {
    EXPECT_CALL(mock_discovery_service, RotatedUriList(_));
    request_handler.PrepareProcessingRequestLocked();
}

TEST_F(RequestHandlerTcpTests, DoesNotGetsUriIfAlreadyConnected) {
    DoSingleSend();
    EXPECT_CALL(mock_discovery_service, RotatedUriList(_)).Times(0);
    request_handler.PrepareProcessingRequestLocked();
}

TEST_F(RequestHandlerTcpTests, ReduceConnectionNumberAtTearDownIfError) {
    auto err = asapo::TextError("error");
    n_connections = 1;

    request_handler.TearDownProcessingRequestLocked(err);

    ASSERT_THAT(n_connections, Eq(0));

}

TEST_F(RequestHandlerTcpTests, DoNotReduceConnectionNumberAtTearDownIfNoError) {
    n_connections = 1;

    request_handler.TearDownProcessingRequestLocked(nullptr);

    ASSERT_THAT(n_connections, Eq(1));
}


TEST_F(RequestHandlerTcpTests, TriesConnectWhenNotConnected) {
    ExpectFailConnect();

    request_handler.PrepareProcessingRequestLocked();
    auto err = request_handler.ProcessRequestUnlocked(&request);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}

TEST_F(RequestHandlerTcpTests, DoesNotTryConnectWhenConnected) {
    DoSingleSend();

    EXPECT_CALL(mock_discovery_service, RotatedUriList(_)).
    WillOnce(Return(receivers_list_single));


    EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(_, _))
    .Times(0);

    ExpectFailSendHeader(true);

    auto err = request_handler.ProcessRequestUnlocked(&request);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}



TEST_F(RequestHandlerTcpTests, DoNotCloseWhenNotConnected) {
    EXPECT_CALL(mock_io, CloseSocket_t(_, _)).Times(0);
    ExpectOKConnect();
    ExpectFailSendHeader();

    request_handler.PrepareProcessingRequestLocked();
    auto err = request_handler.ProcessRequestUnlocked(&request);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}


TEST_F(RequestHandlerTcpTests, CloseConnectionWhenRebalance) {
    DoSingleSend();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_CALL(mock_discovery_service, RotatedUriList(_)).
    WillOnce(Return(asapo::ReceiversList{}));

    EXPECT_CALL(mock_io, CloseSocket_t(_, _));

    auto err = request_handler.ProcessRequestUnlocked(&request);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}



TEST_F(RequestHandlerTcpTests, ErrorWhenCannotSendHeader) {
    ExpectOKConnect();
    ExpectFailSendHeader();

    request_handler.PrepareProcessingRequestLocked();
    auto err = request_handler.ProcessRequestUnlocked(&request);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}


TEST_F(RequestHandlerTcpTests, ErrorWhenCannotSendData) {
    ExpectOKConnect();
    ExpectOKSendHeader();
    ExpectFailSendData();

    request_handler.PrepareProcessingRequestLocked();
    auto err = request_handler.ProcessRequestUnlocked(&request);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}

TEST_F(RequestHandlerTcpTests, ErrorWhenCannotReceiveData) {
    ExpectOKConnect();
    ExpectOKSendHeader();
    ExpectOKSendData();
    ExpectFailReceive();

    request_handler.PrepareProcessingRequestLocked();
    auto err = request_handler.ProcessRequestUnlocked(&request);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}

TEST_F(RequestHandlerTcpTests, ImmediatelyCallBackErrorIfFileAlreadyInUse) {
    ExpectOKConnect(true);
    ExpectOKSendHeader(true);
    ExpectOKSendData(true);

    EXPECT_CALL(mock_io, Receive_t(expected_sds[0], _, sizeof(asapo::SendDataResponse), _))
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(nullptr),
            A_WriteSendDataResponse(asapo::kNetErrorFileIdAlreadyInUse),
            testing::ReturnArg<2>()
        ));


    request_handler.PrepareProcessingRequestLocked();
    auto err = request_handler.ProcessRequestUnlocked(&request);

    ASSERT_THAT(callback_err, Eq(asapo::ProducerErrorTemplates::kFileIdAlreadyInUse));
    ASSERT_THAT(called, Eq(true));
    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(RequestHandlerTcpTests, SendEmptyCallBack) {
    ExpectOKConnect(true);
    ExpectOKSendHeader(true);
    ExpectOKSendData(true);
    ExpectOKReceive();

    request_handler.PrepareProcessingRequestLocked();
    auto err = request_handler.ProcessRequestUnlocked(&request_nocallback);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(called, Eq(false));
}

TEST_F(RequestHandlerTcpTests, SendOK) {
    ExpectOKConnect(true);
    ExpectOKSendHeader(true);
    ExpectOKSendData(true);
    ExpectOKReceive();

    request_handler.PrepareProcessingRequestLocked();
    auto err = request_handler.ProcessRequestUnlocked(&request);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(callback_err, Eq(nullptr));
    ASSERT_THAT(called, Eq(true));
    ASSERT_THAT(callback_header.data_size, Eq(header.data_size));
    ASSERT_THAT(callback_header.op_code, Eq(header.op_code));
    ASSERT_THAT(callback_header.data_id, Eq(header.data_id));
    ASSERT_THAT(std::string{callback_header.file_name}, Eq(std::string{header.file_name}));
}


}
