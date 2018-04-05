#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>

#include "common/error.h"
#include "system/io.h"
#include "producer/producer.h"
#include "../src/producer_impl.h"

namespace {
using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Mock;
using ::testing::InSequence;

TEST(get_version, VersionAboveZero) {
    hidra2::ProducerImpl producer;
    EXPECT_GE(producer.GetVersion(), 0);
}


/**
 * ConnectToReceiver
 */

class ProducerImpl : public testing::Test {
  public:
    hidra2::ProducerImpl producer;
    testing::NiceMock<hidra2::MockIO> mock_io;
    hidra2::FileDescriptor expected_fd = 83942;
    uint64_t expected_file_id = 4224;
    std::string expected_address = "127.0.0.1:9090";
    uint64_t expected_request_id = 0;
    uint64_t expected_file_size = 1337;
    void*    expected_file_pointer = (void*)0xC00FE;

    void SetUp() override {
        producer.io__ = std::unique_ptr<hidra2::IO> {&mock_io};
    }
    void TearDown() override {
        producer.io__.release();
    }

    void ConnectToReceiver_DONE(hidra2::FileDescriptor expected_fd = 1) {
        EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(_, _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(nullptr),
                Return(expected_fd)
            ));
        producer.ConnectToReceiver("");
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
    hidra2::ProducerStatus status = producer.GetStatus();
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kDisconnected));
}


TEST_F(ProducerImpl, ConnectToReceiver__CreateAndConnectIPTCPSocket_error) {
    EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_address, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOErrorTemplates::kInvalidAddressFormat.Generate().release()),
            Return(-1)
        ));

    auto error = producer.ConnectToReceiver(expected_address);
    auto status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::IOErrorTemplates::kInvalidAddressFormat));
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kDisconnected));
}

TEST_F(ProducerImpl, ConnectToReceiver) {
    EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_address, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(nullptr),
            Return(expected_fd)
        ));

    auto error = producer.ConnectToReceiver(expected_address);
    auto status = producer.GetStatus();

    ASSERT_THAT(error, Eq(nullptr));
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kConnected));
}

TEST_F(ProducerImpl, ConnectToReceiver__already_connected) {
    InSequence sequence;

    ConnectToReceiver_DONE();

    auto error = producer.ConnectToReceiver(expected_address);

    ASSERT_THAT(error, Eq(hidra2::ProducerErrorTemplates::kAlreadyConnected));
}

/**
 * Send
 */

MATCHER_P3(M_CheckSendDataRequest, request_id, file_id, file_size,
           "Checks if a valid GenericNetworkRequestHeader was Send") {
    return ((hidra2::GenericNetworkRequestHeader*)arg)->op_code == hidra2::kNetOpcodeSendData
           && ((hidra2::GenericNetworkRequestHeader*)arg)->request_id == request_id
           && ((hidra2::GenericNetworkRequestHeader*)arg)->data_id == file_id
           && ((hidra2::GenericNetworkRequestHeader*)arg)->data_size == file_size;
}

ACTION_P2(A_WriteSendDataResponse, error_code, request_id) {
    ((hidra2::SendDataResponse*)arg1)->op_code = hidra2::kNetOpcodeSendData;
    ((hidra2::SendDataResponse*)arg1)->error_code = error_code;
    ((hidra2::SendDataResponse*)arg1)->request_id = request_id;
}

TEST_F(ProducerImpl, Send__connection_not_ready) {

    auto error = producer.Send(expected_file_id, nullptr, 1);

    ASSERT_THAT(error, Eq(hidra2::ProducerErrorTemplates::kConnectionNotReady));
}

TEST_F(ProducerImpl, Send__file_too_large) {

    ConnectToReceiver_DONE(expected_fd);

    auto error = producer.Send(expected_file_id, nullptr,
                               size_t(1024) * size_t(1024) * size_t(1024) * size_t(3));

    ASSERT_THAT(error, Eq(hidra2::ProducerErrorTemplates::kFileTooLarge));
}

TEST_F(ProducerImpl, Send__sendDataRequest_error) {
    InSequence sequence;

    ConnectToReceiver_DONE(expected_fd);

    EXPECT_CALL(mock_io, Send_t(expected_fd, M_CheckSendDataRequest(expected_request_id, expected_file_id,
                                expected_file_size),
                                sizeof(hidra2::GenericNetworkRequestHeader), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrorTemplates::kBadFileNumber.Generate().release()),
            Return(-1)
        ));

    auto error = producer.Send(expected_file_id, nullptr, expected_file_size);

    ASSERT_THAT(error, Eq(hidra2::IOErrorTemplates::kBadFileNumber));
}

TEST_F(ProducerImpl, Send__sendData_error) {
    InSequence sequence;

    ConnectToReceiver_DONE(expected_fd);
    Send_DONE();

    EXPECT_CALL(mock_io, Send_t(expected_fd, expected_file_pointer, expected_file_size, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrorTemplates::kBadFileNumber.Generate().release()),
            Return(-1)
        ));


    auto error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);

    ASSERT_THAT(error, Eq(hidra2::IOErrorTemplates::kBadFileNumber));
}


TEST_F(ProducerImpl, Send__Receive_error) {
    InSequence sequence;

    ConnectToReceiver_DONE(expected_fd);
    Send_DONE(2);

    EXPECT_CALL(mock_io, Receive_t(expected_fd, _, sizeof(hidra2::SendDataResponse), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrorTemplates::kBadFileNumber.Generate().release()),
            testing::Return(-1)
        ));

    auto error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);

    ASSERT_THAT(error, Eq(hidra2::IOErrorTemplates::kBadFileNumber));
}

TEST_F(ProducerImpl, Send__Receive_server_error) {
    InSequence sequence;

    ConnectToReceiver_DONE(expected_fd);
    Send_DONE(2);


    EXPECT_CALL(mock_io, Receive_t(_, _, sizeof(hidra2::SendDataResponse), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(nullptr),
            A_WriteSendDataResponse(hidra2::kNetErrorAllocateStorageFailed, expected_request_id),
            testing::ReturnArg<2>()
        ));

    auto error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);

    ASSERT_THAT(error, Eq(hidra2::ProducerErrorTemplates::kUnknownServerError));
}

TEST_F(ProducerImpl, Send__Receive_server_error_id_already_in_use) {
    InSequence sequence;

    ConnectToReceiver_DONE(expected_fd);
    Send_DONE(2);


    EXPECT_CALL(mock_io, Receive_t(_, _, sizeof(hidra2::SendDataResponse), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(nullptr),
            A_WriteSendDataResponse(hidra2::kNetErrorFileIdAlreadyInUse, expected_request_id),
            testing::ReturnArg<2>()
        ));

    auto error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);

    ASSERT_THAT(error, Eq(hidra2::ProducerErrorTemplates::kFileIdAlreadyInUse));
}

TEST_F(ProducerImpl, Send) {
    InSequence sequence;

    ConnectToReceiver_DONE(expected_fd);
    Send_DONE(2);


    EXPECT_CALL(mock_io, Receive_t(_, _, sizeof(hidra2::SendDataResponse), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(nullptr),
            A_WriteSendDataResponse(hidra2::kNetErrorNoError, expected_request_id),
            testing::ReturnArg<2>()
        ));

    auto error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);

    ASSERT_THAT(error, Eq(nullptr));
}

}
