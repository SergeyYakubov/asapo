#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"
#include "common/error.h"
#include "io/io.h"

#include "producer/producer.h"
#include "producer/producer_error.h"

#include "../src/request.h"
#include <common/networking.h>
#include "io/io_factory.h"

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


using ::testing::InSequence;
using ::testing::HasSubstr;


TEST(Producer, Constructor) {
    auto  io = std::unique_ptr<asapo::IO> {asapo::GenerateDefaultIO()};
    asapo::GenericNetworkRequestHeader header;
    asapo::Request request{io.get(), header, nullptr, [](asapo::GenericNetworkRequestHeader, asapo::Error) {}};

    ASSERT_THAT(dynamic_cast<const asapo::IO*>(request.io__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(request.log__), Ne(nullptr));

}


class RequestTests : public testing::Test {
  public:
    testing::NiceMock<asapo::MockIO> mock_io;
    uint64_t expected_file_id = 4224;
    uint64_t expected_file_size = 1337;
    asapo::Opcode expected_op_code = asapo::kNetOpcodeSendData;
    void*    expected_file_pointer = (void*)0xC00FE;
    asapo::Error callback_err;

    asapo::GenericNetworkRequestHeader header{expected_op_code, expected_file_id, expected_file_size};
    bool called = false;
    asapo::GenericNetworkRequestHeader callback_header;
    asapo::Request request{&mock_io, header, expected_file_pointer, [this](asapo::GenericNetworkRequestHeader header, asapo::Error err) {
        called = true;
        callback_err = std::move(err);
        callback_header = header;
    }};
    testing::NiceMock<asapo::MockLogger> mock_logger;

    asapo::SocketDescriptor sd = asapo::kDisconnectedSocketDescriptor;
    std::string expected_address1 = {"127.0.0.1:9090"};
    std::string expected_address2 = {"127.0.0.1:9091"};
    asapo::ReceiversList receivers_list{expected_address1, expected_address2};
    std::vector<asapo::SocketDescriptor> expected_sds{83942, 83943};

    void ExpectFailConnect(bool only_once = false);
    void ExpectFailSendHeader(bool only_once = false);
    void ExpectFailSendData(bool only_once = false);
    void ExpectOKConnect(bool only_once = false);
    void ExpectOKSendHeader(bool only_once = false);
    void ExpectOKSendData(bool only_once = false);
    void ExpectFailReceive(bool only_once = false);
    void ExpectOKReceive(bool only_once = true);

    void SetUp() override {
        request.log__ = &mock_logger;
    }
    void TearDown() override {
    }
};

ACTION_P(A_WriteSendDataResponse, error_code) {
    ((asapo::SendDataResponse*)arg1)->op_code = asapo::kNetOpcodeSendData;
    ((asapo::SendDataResponse*)arg1)->error_code = error_code;
}



MATCHER_P2(M_CheckSendDataRequest, file_id, file_size,
           "Checks if a valid GenericNetworkRequestHeader was Send") {
    return ((asapo::GenericNetworkRequestHeader*)arg)->op_code == asapo::kNetOpcodeSendData
           && ((asapo::GenericNetworkRequestHeader*)arg)->data_id == file_id
           && ((asapo::GenericNetworkRequestHeader*)arg)->data_size == file_size;
}

void RequestTests::ExpectFailConnect(bool only_once) {
    for (auto expected_address : receivers_list) {
        EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_address, _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(asapo::IOErrorTemplates::kInvalidAddressFormat.Generate().release()),
                Return(asapo::kDisconnectedSocketDescriptor)
            ));
        if (only_once) break;
    }

}


void RequestTests::ExpectFailSendHeader(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, M_CheckSendDataRequest(expected_file_id,
                                    expected_file_size),
                                    sizeof(asapo::GenericNetworkRequestHeader), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
                Return(-1)
            ));
        EXPECT_CALL(mock_logger, Debug(AllOf(
            HasSubstr("cannot send header"),
            HasSubstr(receivers_list[i])
                                       )
        ));
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) break;
        i++;
    }

}

void RequestTests::ExpectFailSendData(bool only_once) {
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
            HasSubstr("cannot send data"),
            HasSubstr(receivers_list[i])
                                       )
        ));
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) break;
        i++;
    }

}


void RequestTests::ExpectFailReceive(bool only_once) {
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
            HasSubstr("cannot receive"),
            HasSubstr(receivers_list[i])
                                      )
        ));
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) break;
        i++;
    }

}


void RequestTests::ExpectOKSendData(bool only_once) {
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



void RequestTests::ExpectOKSendHeader(bool only_once) {
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, M_CheckSendDataRequest(expected_file_id,
                                    expected_file_size),
                                    sizeof(asapo::GenericNetworkRequestHeader), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                Return(sizeof(asapo::GenericNetworkRequestHeader))
            ));
        if (only_once) break;
    }

}


void RequestTests::ExpectOKConnect(bool only_once) {
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


void RequestTests::ExpectOKReceive(bool only_once) {
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


TEST_F(RequestTests, TriesConnectWhenNotConnected) {
    ExpectFailConnect();

    auto err = request.Send(&sd, receivers_list);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}

TEST_F(RequestTests, DoesNotTryConnectWhenConnected) {
    sd = expected_sds[0];
    EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(_, _))
    .Times(0);
    ExpectFailSendHeader(true);


    auto err = request.Send(&sd, asapo::ReceiversList{expected_address1});

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}



TEST_F(RequestTests, ErrorWhenCannotSendHeader) {
    ExpectOKConnect();
    ExpectFailSendHeader();

    auto err = request.Send(&sd, receivers_list);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}

TEST_F(RequestTests, ErrorWhenCannotSendData) {
    ExpectOKConnect();
    ExpectOKSendHeader();
    ExpectFailSendData();

    auto err = request.Send(&sd, receivers_list);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}

TEST_F(RequestTests, ErrorWhenCannotReceiveData) {
    ExpectOKConnect();
    ExpectOKSendHeader();
    ExpectOKSendData();

    ExpectFailReceive();

    auto err = request.Send(&sd, receivers_list);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}




TEST_F(RequestTests, ImmediatelyCalBackErrorIfFileAlreadyInUse) {
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


    auto err = request.Send(&sd, receivers_list);

    ASSERT_THAT(callback_err, Eq(asapo::ProducerErrorTemplates::kFileIdAlreadyInUse));
    ASSERT_THAT(called, Eq(true));
    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(RequestTests, SendOK) {
    ExpectOKConnect(true);
    ExpectOKSendHeader(true);
    ExpectOKSendData(true);
    ExpectOKReceive();

    auto err = request.Send(&sd, receivers_list);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(sd, Eq(expected_sds[0]));
    ASSERT_THAT(callback_err, Eq(nullptr));
    ASSERT_THAT(called, Eq(true));
    ASSERT_THAT(callback_header.data_size, Eq(header.data_size));
    ASSERT_THAT(callback_header.op_code, Eq(header.op_code));
    ASSERT_THAT(callback_header.data_id, Eq(header.data_id));

}



}
