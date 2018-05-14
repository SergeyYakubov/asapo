#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"
#include "common/error.h"
#include "io/io.h"
#include "producer/producer.h"
#include "../src/producer_impl.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::InSequence;
using ::testing::HasSubstr;

TEST(get_version, VersionAboveZero) {
    asapo::ProducerImpl producer{4};
    EXPECT_GE(producer.GetVersion(), 0);
}


TEST(Producer, Logger) {
    asapo::ProducerImpl producer{4};
    ASSERT_THAT(dynamic_cast<asapo::AbstractLogger*>(producer.log__), Ne(nullptr));
}

/*
class ProducerImpl : public testing::Test {
  public:
    asapo::ProducerImpl producer;
    testing::NiceMock<asapo::MockIO> mock_io;
    testing::NiceMock<asapo::MockLogger> mock_logger;

    asapo::FileDescriptor expected_fd = 83942;
    uint64_t expected_file_id = 4224;
    std::string expected_address = "127.0.0.1:9090";
    uint64_t expected_request_id = 0;
    uint64_t expected_file_size = 1337;
    void*    expected_file_pointer = (void*)0xC00FE;

    void SetUp() override {
        producer.io__ = std::unique_ptr<asapo::IO> {&mock_io};
        producer.log__ = asapo::Logger {&mock_logger};
    }
    void TearDown() override {
        producer.io__.release();
        producer.log__.release();
    }

    void ConnectToReceiver_DONE(asapo::FileDescriptor expected_fd = 1) {
        EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_address, _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(nullptr),
                Return(expected_fd)
            ));
        producer.ConnectToReceiver(expected_address);
    }
    void Send_DONE(int times = 1) {
        EXPECT_CALL(mock_io, Send_t(_, _, _, _))
        .Times(times)
        .WillRepeatedly(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                testing::ReturnArg<2>()
            ));
    }
};

TEST_F(ProducerImpl, get_status__disconnected) {
    asapo::ProducerStatus status = producer.GetStatus();
    ASSERT_THAT(status, Eq(asapo::ProducerStatus::kDisconnected));
}


TEST_F(ProducerImpl, ConnectToReceiver__CreateAndConnectIPTCPSocket_error) {
    EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_address, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(asapo::IOErrorTemplates::kInvalidAddressFormat.Generate().release()),
            Return(-1)
        ));

    EXPECT_CALL(mock_logger, Debug(HasSubstr("cannot connect")));

    auto error = producer.ConnectToReceiver(expected_address);
    auto status = producer.GetStatus();

    ASSERT_THAT(error, Eq(asapo::IOErrorTemplates::kInvalidAddressFormat));
    ASSERT_THAT(status, Eq(asapo::ProducerStatus::kDisconnected));
}

TEST_F(ProducerImpl, ConnectToReceiver) {
    EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_address, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(nullptr),
            Return(expected_fd)
        ));

    EXPECT_CALL(mock_logger, Info(HasSubstr("connected")));


    auto error = producer.ConnectToReceiver(expected_address);
    auto status = producer.GetStatus();

    ASSERT_THAT(error, Eq(nullptr));
    ASSERT_THAT(status, Eq(asapo::ProducerStatus::kConnected));
}

TEST_F(ProducerImpl, ConnectToReceiver__already_connected) {
    InSequence sequence;

    ConnectToReceiver_DONE();

    auto error = producer.ConnectToReceiver(expected_address);

    ASSERT_THAT(error, Eq(asapo::ProducerErrorTemplates::kAlreadyConnected));
}



MATCHER_P2(M_CheckSendDataRequest, file_id, file_size,
           "Checks if a valid GenericNetworkRequestHeader was Send") {
    return ((asapo::GenericNetworkRequestHeader*)arg)->op_code == asapo::kNetOpcodeSendData
           && ((asapo::GenericNetworkRequestHeader*)arg)->data_id == file_id
           && ((asapo::GenericNetworkRequestHeader*)arg)->data_size == file_size;
}

ACTION_P2(A_WriteSendDataResponse, error_code, request_id) {
    ((asapo::SendDataResponse*)arg1)->op_code = asapo::kNetOpcodeSendData;
    ((asapo::SendDataResponse*)arg1)->error_code = error_code;
    ((asapo::SendDataResponse*)arg1)->request_id = request_id;
}

TEST_F(ProducerImpl, Send__connection_not_ready) {

    auto error = producer.Send(expected_file_id, nullptr, 1);

    ASSERT_THAT(error, Eq(asapo::ProducerErrorTemplates::kConnectionNotReady));
}

TEST_F(ProducerImpl, Send__file_too_large) {

    ConnectToReceiver_DONE(expected_fd);

    auto error = producer.Send(expected_file_id, nullptr,
                               size_t(1024) * size_t(1024) * size_t(1024) * size_t(3));

    ASSERT_THAT(error, Eq(asapo::ProducerErrorTemplates::kFileTooLarge));
}
/*
TEST_F(ProducerImpl, Send__sendDataRequest_error) {
    InSequence sequence;

    ConnectToReceiver_DONE(expected_fd);

    EXPECT_CALL(mock_io, Send_t(expected_fd, M_CheckSendDataRequest(expected_file_id,
                                expected_file_size),
                                sizeof(asapo::GenericNetworkRequestHeader), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
            Return(-1)
        ));

    auto error = producer.Send(expected_file_id, nullptr, expected_file_size);

    ASSERT_THAT(error, Eq(asapo::IOErrorTemplates::kBadFileNumber));
}

TEST_F(ProducerImpl, Send__sendData_error) {
    InSequence sequence;

    ConnectToReceiver_DONE(expected_fd);
    Send_DONE();

    EXPECT_CALL(mock_io, Send_t(expected_fd, expected_file_pointer, expected_file_size, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
            Return(-1)
        ));

    EXPECT_CALL(mock_logger, Debug(HasSubstr("error sending to " + expected_address)));

    auto error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);

    ASSERT_THAT(error, Eq(asapo::IOErrorTemplates::kBadFileNumber));
}


TEST_F(ProducerImpl, Send__Receive_error) {
    InSequence sequence;

    ConnectToReceiver_DONE(expected_fd);
    Send_DONE(2);

    EXPECT_CALL(mock_io, Receive_t(expected_fd, _, sizeof(asapo::SendDataResponse), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
            testing::Return(-1)
        ));

    EXPECT_CALL(mock_logger, Debug(HasSubstr("error receiving response from " + expected_address)));

    auto error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);

    ASSERT_THAT(error, Eq(asapo::IOErrorTemplates::kBadFileNumber));
}

TEST_F(ProducerImpl, Send__Receive_server_error) {
    InSequence sequence;

    ConnectToReceiver_DONE(expected_fd);
    Send_DONE(2);


    EXPECT_CALL(mock_io, Receive_t(_, _, sizeof(asapo::SendDataResponse), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(nullptr),
            A_WriteSendDataResponse(asapo::kNetErrorAllocateStorageFailed, expected_request_id),
            testing::ReturnArg<2>()
        ));

    auto error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);

    ASSERT_THAT(error, Eq(asapo::ProducerErrorTemplates::kUnknownServerError));
}

TEST_F(ProducerImpl, Send__Receive_server_error_id_already_in_use) {
    InSequence sequence;

    ConnectToReceiver_DONE(expected_fd);
    Send_DONE(2);


    EXPECT_CALL(mock_io, Receive_t(_, _, sizeof(asapo::SendDataResponse), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(nullptr),
            A_WriteSendDataResponse(asapo::kNetErrorFileIdAlreadyInUse, expected_request_id),
            testing::ReturnArg<2>()
        ));

    auto error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);

    ASSERT_THAT(error, Eq(asapo::ProducerErrorTemplates::kFileIdAlreadyInUse));
}

TEST_F(ProducerImpl, Send) {
    InSequence sequence;

    ConnectToReceiver_DONE(expected_fd);
    Send_DONE(2);


    EXPECT_CALL(mock_io, Receive_t(_, _, sizeof(asapo::SendDataResponse), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(nullptr),
            A_WriteSendDataResponse(asapo::kNetErrorNoError, expected_request_id),
            testing::ReturnArg<2>()
        ));

    EXPECT_CALL(mock_logger, Debug(HasSubstr("succesfully sent data to " + expected_address)));

    auto error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);

    ASSERT_THAT(error, Eq(nullptr));
}

TEST_F(ProducerImpl, EnableLocalLog) {

    EXPECT_CALL(mock_logger, EnableLocalLog(true));

    producer.EnableLocalLog(true);

}

TEST_F(ProducerImpl, EnableRemoteLog) {

    EXPECT_CALL(mock_logger, EnableRemoteLog(false));

    producer.EnableRemoteLog(false);

}


TEST_F(ProducerImpl, SetLogLevel) {

    EXPECT_CALL(mock_logger, SetLogLevel(asapo::LogLevel::Warning));

    producer.SetLogLevel(asapo::LogLevel::Warning);

}


 */
}
