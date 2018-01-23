#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../src/producer_impl.h"
#include "../../../common/cpp/unittests/MockIO.h"

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

TEST(ProducerImpl, get_status__disconnected) {
    hidra2::ProducerImpl producer;

    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kDisconnected));
}

/**
 * ConnectToReceiver
 */

TEST(ProducerImpl, ConnectToReceiver__CreateAndConnectIPTCPSocket_invalid_address_format) {
    hidra2::ProducerImpl producer;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    std::string expected_address = "127.0.0.1:9090";

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(expected_address, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOErrors::kInvalidAddressFormat),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.ConnectToReceiver(expected_address);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::ProducerError::kInvalidAddressFormat));
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kDisconnected));
}

TEST(ProducerImpl, ConnectToReceiver__CreateAndConnectIPTCPSocket_unk) {
    hidra2::ProducerImpl producer;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    std::string expected_address = "127.0.0.1:9090";

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(expected_address, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOErrors::kUnknownError),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.ConnectToReceiver(expected_address);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::ProducerError::kUnknownError));
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kDisconnected));
}

TEST(ProducerImpl, ConnectToReceiver) {
    hidra2::ProducerImpl producer;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    std::string expected_address = "127.0.0.1:9090";
    hidra2::FileDescriptor expected_fd = 199;

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(expected_address, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOErrors::kNoError),
            Return(expected_fd)
        ));

    hidra2::ProducerError error = producer.ConnectToReceiver(expected_address);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::ProducerError::kNoError));
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kConnected));
}

void ConnectToReceiver_DONE(hidra2::ProducerImpl& producer, hidra2::FileDescriptor expected_fd = 1) {
    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(_, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOErrors::kNoError),
            Return(expected_fd)
        ));

    producer.ConnectToReceiver("");
}

TEST(ProducerImpl, ConnectToReceiver__already_connected) {
    hidra2::ProducerImpl    producer;
    std::string             expected_address = "127.0.0.1:9090";


    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    InSequence sequence;

    ConnectToReceiver_DONE(producer);

    hidra2::ProducerError error = producer.ConnectToReceiver(expected_address);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::ProducerError::kAlreadyConnected));
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kConnected));
}

/**
 * Send
 */

MATCHER_P3(M_CheckSendDataRequest, request_id, file_id, file_size,
           "Checks if a valid SendDataRequest was Send") {
    return ((hidra2::SendDataRequest*)arg)->op_code == hidra2::OP_CODE__SEND_DATA
           && ((hidra2::SendDataRequest*)arg)->request_id == request_id
           && ((hidra2::SendDataRequest*)arg)->file_id, file_id
           && ((hidra2::SendDataRequest*)arg)->file_size == file_size;
}

ACTION_P2(A_WriteSendDataResponse, error_code, request_id) {
    ((hidra2::SendDataResponse*)arg1)->op_code = hidra2::OP_CODE__SEND_DATA;
    ((hidra2::SendDataResponse*)arg1)->error_code = error_code;
    ((hidra2::SendDataResponse*)arg1)->request_id = request_id;
}

TEST(ProducerImpl, Send__connection_not_ready) {
    hidra2::ProducerImpl producer;
    uint64_t expected_file_id = 4224;

    hidra2::ProducerError error = producer.Send(expected_file_id, nullptr, 1);

    ASSERT_THAT(error, Eq(hidra2::ProducerError::kConnectionNotReady));
}

TEST(ProducerImpl, Send__file_too_large) {
    hidra2::ProducerImpl producer;
    hidra2::FileDescriptor expected_fd = 83942;
    uint64_t expected_file_id = 4224;

    ConnectToReceiver_DONE(producer, expected_fd);

    hidra2::ProducerError error = producer.Send(expected_file_id, nullptr,
                                                size_t(1024) * size_t(1024) * size_t(1024) * size_t(3));
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::ProducerError::kFileTooLarge));
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kConnected));
}

TEST(ProducerImpl, Send__sendDataRequest_error) {
    InSequence sequence;

    hidra2::ProducerImpl producer;
    hidra2::FileDescriptor expected_fd = 83942;
    uint64_t expected_request_id = 0;
    uint64_t expected_file_id = 1;
    uint64_t expected_file_size = 1337;

    ConnectToReceiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(expected_fd, M_CheckSendDataRequest(expected_request_id, expected_file_id, expected_file_size),
                             sizeof(hidra2::SendDataRequest), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrors::kBadFileNumber),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.Send(expected_file_id, nullptr, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::ProducerError::kUnexpectedIOError));
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kConnected));
}

TEST(ProducerImpl, Send__sendData_error) {
    InSequence sequence;

    hidra2::ProducerImpl producer;
    hidra2::FileDescriptor expected_fd = 83942;
    uint64_t expected_request_id = 0;
    uint64_t expected_file_id = 1;
    uint64_t expected_file_size = 1337;
    void*    expected_file_pointer = (void*)0xC00FE;

    ConnectToReceiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrors::kNoError),
            testing::ReturnArg<2>()
        ));

    EXPECT_CALL(mockIO, Send(expected_fd, expected_file_pointer, expected_file_size, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrors::kBadFileNumber),
            Return(-1)
        ));


    hidra2::ProducerError error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::ProducerError::kUnexpectedIOError));
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kConnected));
}


TEST(ProducerImpl, Send__Receive_error) {
    InSequence sequence;

    hidra2::ProducerImpl producer;
    hidra2::FileDescriptor expected_fd = 83942;
    uint64_t expected_request_id = 0;
    uint64_t expected_file_id = 1;
    uint64_t expected_file_size = 1337;
    void*    expected_file_pointer = (void*)0xC00FE;

    ConnectToReceiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(2)
    .WillRepeatedly(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrors::kNoError),
            testing::ReturnArg<2>()
        ));

    EXPECT_CALL(mockIO, Receive(expected_fd, _, sizeof(hidra2::SendDataResponse), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrors::kBadFileNumber),
            testing::Return(-1)
        ));

    hidra2::ProducerError error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::ProducerError::kUnexpectedIOError));
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kConnected));
}

TEST(ProducerImpl, Send__Receive_server_error) {
    InSequence sequence;

    hidra2::ProducerImpl producer;
    hidra2::FileDescriptor expected_fd = 83942;
    uint64_t expected_request_id = 0;
    uint64_t expected_file_id = 1;
    uint64_t expected_file_size = 1337;
    void*    expected_file_pointer = (void*)0xC00FE;

    ConnectToReceiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(2)
    .WillRepeatedly(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrors::kNoError),
            testing::ReturnArg<2>()
        ));

    EXPECT_CALL(mockIO, Receive(_, _, sizeof(hidra2::SendDataResponse), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrors::kNoError),
            A_WriteSendDataResponse(hidra2::NET_ERR__ALLOCATE_STORAGE_FAILED, expected_request_id),
            testing::ReturnArg<2>()
        ));

    hidra2::ProducerError error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::ProducerError::kUnknownServerError));
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kConnected));
}


TEST(ProducerImpl, Send) {
    InSequence sequence;

    hidra2::ProducerImpl producer;
    hidra2::FileDescriptor expected_fd = 83942;
    uint64_t expected_request_id = 0;
    uint64_t expected_file_id = 1;
    uint64_t expected_file_size = 1337;
    void*    expected_file_pointer = (void*)0xC00FE;

    ConnectToReceiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(2)
    .WillRepeatedly(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrors::kNoError),
            testing::ReturnArg<2>()
        ));

    EXPECT_CALL(mockIO, Receive(_, _, sizeof(hidra2::SendDataResponse), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrors::kNoError),
            A_WriteSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id),
            testing::ReturnArg<2>()
        ));

    hidra2::ProducerError error = producer.Send(expected_file_id, expected_file_pointer, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::ProducerError::kNoError));
    ASSERT_THAT(status, Eq(hidra2::ProducerStatus::kConnected));
}

}
