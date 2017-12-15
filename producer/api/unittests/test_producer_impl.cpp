#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <netinet/in.h>
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

    ASSERT_THAT(status, Eq(hidra2::PRODUCER_STATUS__DISCONNECTED));
}

/**
 * connect_to_receiver
 */

MATCHER_P(M_CheckHelloRequest, request_id, "Checks if a valid HelloRequest was Send") {
    return ((hidra2::HelloRequest*)arg)->op_code == hidra2::OP_CODE__HELLO
           && ((hidra2::HelloRequest*)arg)->request_id == request_id;
}

ACTION_P3(A_WriteHelloResponse, error_code, request_id, server_version) {
    ((hidra2::HelloResponse*)arg1)->op_code = hidra2::OP_CODE__HELLO;
    ((hidra2::HelloResponse*)arg1)->error_code = error_code;
    ((hidra2::HelloResponse*)arg1)->request_id = request_id;
    ((hidra2::HelloResponse*)arg1)->server_version = server_version;
}

TEST(ProducerImpl, connect_to_receiver__CreateAndConnectIPTCPSocket_invalid__address_format) {
    hidra2::ProducerImpl producer;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    std::string expected_address = "127.0.0.1:9090";

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(expected_address, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOError::INVALID_ADDRESS_FORMAT),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.ConnectToReceiver(expected_address);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::PRODUCER_ERROR__INVALID_ADDRESS_FORMAT));
    ASSERT_THAT(status, Eq(hidra2::PRODUCER_STATUS__DISCONNECTED));
}

TEST(ProducerImpl, connect_to_receiver__CreateAndConnectIPTCPSocket_failed_unk) {
    hidra2::ProducerImpl producer;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    std::string expected_address = "127.0.0.1:9090";

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(expected_address, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOError::UNKNOWN_ERROR),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.ConnectToReceiver(expected_address);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER));
    ASSERT_THAT(status, Eq(hidra2::PRODUCER_STATUS__DISCONNECTED));
}

TEST(ProducerImpl, connect_to_receiver__Send1_data_check) {
    hidra2::ProducerImpl    producer;
    uint64_t                expected_request_id = 0;
    int                     expected_fd         = 83942;


    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    InSequence sequence;

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(_, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOError::NO_ERROR),
            Return(expected_fd)
        ));

    EXPECT_CALL(mockIO, Send(expected_fd, M_CheckHelloRequest(expected_request_id), sizeof(hidra2::HelloRequest), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    producer.ConnectToReceiver("");
}

TEST(ProducerImpl, connect_to_receiver__Send1_io_error) {
    hidra2::ProducerImpl    producer;


    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    InSequence sequence;

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(_, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOError::NO_ERROR),
            Return(1)
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.ConnectToReceiver("");
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER));
    ASSERT_THAT(status, Eq(hidra2::PRODUCER_STATUS__DISCONNECTED));
}

TEST(ProducerImpl, connect_to_receiver__receive1_data_check) {
    hidra2::ProducerImpl    producer;
    uint64_t                expected_request_id = 0;
    int                     expected_fd         = 83942;


    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    InSequence sequence;

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(_, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOError::NO_ERROR),
            Return(expected_fd)
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::HelloRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(expected_fd, _/*addr*/, sizeof(hidra2::HelloResponse), Gt(0), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<4>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.ConnectToReceiver("");
}

TEST(ProducerImpl, connect_to_receiver__receive1_io_error) {
    hidra2::ProducerImpl    producer;
    uint64_t                expected_request_id = 0;
    int                     expected_fd         = 83942;


    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    InSequence sequence;

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(_, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOError::NO_ERROR),
            Return(expected_fd)
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::HelloRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<4>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.ConnectToReceiver("");
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER));
    ASSERT_THAT(status, Eq(hidra2::PRODUCER_STATUS__DISCONNECTED));
}

TEST(ProducerImpl, connect_to_receiver__receive1_server_error) {
    hidra2::ProducerImpl    producer;
    uint64_t                expected_request_id = 0;
    int                     expected_fd         = 83942;


    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    InSequence sequence;

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(_, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOError::NO_ERROR),
            Return(expected_fd)
        ));

    EXPECT_CALL(mockIO, Send(expected_fd, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::HelloRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WriteHelloResponse(hidra2::NET_ERR__UNSUPPORTED_VERSION, expected_request_id, 1),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::HelloResponse))
        ));

    hidra2::ProducerError error = producer.ConnectToReceiver("");
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER));
    ASSERT_THAT(status, Eq(hidra2::PRODUCER_STATUS__DISCONNECTED));
}

TEST(ProducerImpl, connect_to_receiver) {
    hidra2::ProducerImpl    producer;
    uint64_t                expected_request_id = 0;


    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    InSequence sequence;

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(_, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOError::NO_ERROR),
            Return(1)
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::HelloRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WriteHelloResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 1),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::HelloResponse))
        ));

    hidra2::ProducerError error = producer.ConnectToReceiver("");
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::PRODUCER_ERROR__NO_ERROR));
    ASSERT_THAT(status, Eq(hidra2::PRODUCER_STATUS__CONNECTED));
}

void connect_to_receiver_DONE(hidra2::Producer& producer, hidra2::FileDescriptor expected_fd = 1) {
    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, CreateAndConnectIPTCPSocket(_, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOError::NO_ERROR),
            Return(expected_fd)
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::HelloRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WriteHelloResponse(hidra2::NET_ERR__NO_ERROR, 0, 1),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::HelloResponse))
        ));

    producer.ConnectToReceiver("");
}

TEST(ProducerImpl, connect_to_receiver__already_connected) {
    hidra2::ProducerImpl    producer;
    uint64_t                expected_request_id = 0;
    std::string             expected_address    = "127.0.0.1:9090";


    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    InSequence sequence;

    connect_to_receiver_DONE(producer);

    hidra2::ProducerError error = producer.ConnectToReceiver("");
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, Eq(hidra2::PRODUCER_ERROR__ALREADY_CONNECTED));
    ASSERT_THAT(status, Eq(hidra2::PRODUCER_STATUS__CONNECTED));
}

/**
 * Send
 */

MATCHER_P3(M_CheckPrepareSendDataRequest, request_id, filename, file_size,
           "Checks if a valid PrepareSendDataRequest was Send") {
    return ((hidra2::PrepareSendDataRequest*)arg)->op_code == hidra2::OP_CODE__PREPARE_SEND_DATA
           && ((hidra2::PrepareSendDataRequest*)arg)->request_id == request_id
           && strcmp(((hidra2::PrepareSendDataRequest*)arg)->filename, filename.c_str()) == 0
           && ((hidra2::PrepareSendDataRequest*)arg)->file_size == file_size;
}

ACTION_P3(A_WritePrepareSendDataResponse, error_code, request_id, file_reference_id) {
    ((hidra2::PrepareSendDataResponse*)arg1)->op_code = hidra2::OP_CODE__PREPARE_SEND_DATA;
    ((hidra2::PrepareSendDataResponse*)arg1)->error_code = error_code;
    ((hidra2::PrepareSendDataResponse*)arg1)->request_id = request_id;
    ((hidra2::PrepareSendDataResponse*)arg1)->file_reference_id = file_reference_id;
}

MATCHER_P4(M_CheckSendDataChunkRequest, request_id, file_reference_id, chunk_size, start_byte,
           "Checks if a valid SendDataChunkRequest was Send") {
    bool ret = ((hidra2::SendDataChunkRequest*)arg)->op_code == hidra2::OP_CODE__SEND_DATA_CHUNK
               && ((hidra2::SendDataChunkRequest*)arg)->request_id == request_id
               && ((hidra2::SendDataChunkRequest*)arg)->file_reference_id == file_reference_id
               && ((hidra2::SendDataChunkRequest*)arg)->chunk_size == chunk_size
               && ((hidra2::SendDataChunkRequest*)arg)->start_byte == start_byte;
    return ret;
}

ACTION_P2(A_WriteSendDataChunkResponse, error_code, request_id) {
    ((hidra2::SendDataChunkResponse*)arg1)->op_code = hidra2::OP_CODE__SEND_DATA_CHUNK;
    ((hidra2::SendDataChunkResponse*)arg1)->error_code = error_code;
    ((hidra2::SendDataChunkResponse*)arg1)->request_id = request_id;
}

TEST(ProducerImpl, Send__connection_not_ready) {
    hidra2::ProducerImpl producer;

    hidra2::ProducerError error = producer.Send("", nullptr, 1);

    ASSERT_THAT(error, Eq(hidra2::PRODUCER_ERROR__CONNECTION_NOT_READY));
}

TEST(ProducerImpl, Send__SEND_pre_data_check) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    std::string             expected_filename = "test.bin";
    uint64_t                expected_file_size = 1337;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(expected_fd, M_CheckPrepareSendDataRequest(expected_request_id, expected_filename,
                             expected_file_size), sizeof(hidra2::PrepareSendDataRequest), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    producer.Send(expected_filename, nullptr, expected_file_size);
}

TEST(ProducerImpl, Send__SEND_pre_io_error) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;

    connect_to_receiver_DONE(producer);

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.Send("", nullptr, 1);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, hidra2::PRODUCER_ERROR__SENDING_SERVER_REQUEST_FAILED);
    ASSERT_THAT(status, hidra2::PRODUCER_STATUS__CONNECTED);
}

TEST(ProducerImpl, Send__receive_pre_io_error) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;

    connect_to_receiver_DONE(producer);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<4>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.Send("", nullptr, 1);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, hidra2::PRODUCER_ERROR__RECEIVING_SERVER_RESPONSE_FAILED);
    ASSERT_THAT(status, hidra2::PRODUCER_STATUS__CONNECTED);
}

TEST(ProducerImpl, Send__receive_pre_server_error) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;

    connect_to_receiver_DONE(producer);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__ALLOCATE_STORAGE_FAILED, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.Send("", nullptr, 1);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, hidra2::PRODUCER_ERROR__SERVER_REPORTED_AN_ERROR);
    ASSERT_THAT(status, hidra2::PRODUCER_STATUS__CONNECTED);
}


TEST(ProducerImpl, Send__SEND_chunk1_meta_data_check) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size = producer.kMaxChunkSize + 1543;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, expected_file_ref_id),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataResponse))
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(expected_fd, M_CheckSendDataChunkRequest(expected_request_id, expected_file_ref_id,
                             producer.kMaxChunkSize, 0), sizeof(hidra2::SendDataChunkRequest), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    producer.Send("", nullptr, expected_file_size);
}

TEST(ProducerImpl, Send__SEND_chunk1_meta_io_error) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size = producer.kMaxChunkSize + 1543;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));


    hidra2::ProducerError error = producer.Send("", nullptr, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, hidra2::PRODUCER_ERROR__SENDING_CHUNK_FAILED);
    ASSERT_THAT(status, hidra2::PRODUCER_STATUS__CONNECTED);
}

TEST(ProducerImpl, Send__SEND_chunk1_data_data_check) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size = producer.kMaxChunkSize + 1543;
    void*                   expected_file_buffer = (void*)0xC0FFEE;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(expected_fd, expected_file_buffer, producer.kMaxChunkSize, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    producer.Send("", expected_file_buffer, expected_file_size);
}

TEST(ProducerImpl, Send__SEND_chunk1_data_io_error) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size = producer.kMaxChunkSize + 1543;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.Send("", nullptr, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, hidra2::PRODUCER_ERROR__SENDING_CHUNK_FAILED);
    ASSERT_THAT(status, hidra2::PRODUCER_STATUS__CONNECTED);
}

TEST(ProducerImpl, Send__receive_chunk1_meta_data_check) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size = producer.kMaxChunkSize + 1543;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(expected_fd, _/*addr*/, sizeof(hidra2::SendDataChunkResponse), Gt(0), _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    producer.Send("", nullptr, expected_file_size);
}

TEST(ProducerImpl, Send__receive_chunk1_meta_io_error) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size = producer.kMaxChunkSize + 1543;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(expected_fd, _/*addr*/, sizeof(hidra2::SendDataChunkResponse), Gt(0), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<4>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.Send("", nullptr, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, hidra2::PRODUCER_ERROR__RECEIVING_SERVER_RESPONSE_FAILED);
    ASSERT_THAT(status, hidra2::PRODUCER_STATUS__CONNECTED);
}

TEST(ProducerImpl, Send__receive_chunk1_meta_server_error) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size = producer.kMaxChunkSize + 1543;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(expected_fd, _/*addr*/, sizeof(hidra2::SendDataChunkResponse), Gt(0), _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__INTERNAL_SERVER_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkResponse))
        ));

    hidra2::ProducerError error = producer.Send("", nullptr, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, hidra2::PRODUCER_ERROR__SERVER_REPORTED_AN_ERROR);
    ASSERT_THAT(status, hidra2::PRODUCER_STATUS__CONNECTED);
}


TEST(ProducerImpl, Send__SEND_chunk2_meta_data_check) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size_overhead = 1543;
    size_t                  expected_file_size = producer.kMaxChunkSize + expected_file_size_overhead;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, expected_file_ref_id),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataResponse))
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkResponse))
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(expected_fd, M_CheckSendDataChunkRequest(expected_request_id, expected_file_ref_id,
                             expected_file_size_overhead, producer.kMaxChunkSize), sizeof(hidra2::SendDataChunkRequest), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    producer.Send("", nullptr, expected_file_size);
}

TEST(ProducerImpl, Send__SEND_chunk2_meta_io_error) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size_overhead = 1543;
    size_t                  expected_file_size = producer.kMaxChunkSize + expected_file_size_overhead;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;
    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkResponse))
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));


    hidra2::ProducerError error = producer.Send("", nullptr, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, hidra2::PRODUCER_ERROR__SENDING_CHUNK_FAILED);
    ASSERT_THAT(status, hidra2::PRODUCER_STATUS__CONNECTED);
}

TEST(ProducerImpl, Send__SEND_chunk2_data_data_check) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size_overhead = 1543;
    size_t                  expected_file_size = producer.kMaxChunkSize + expected_file_size_overhead;
    void*                   expected_file_buffer = (void*)0xC0FFEE;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;
    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkResponse))
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(expected_fd, (uint8_t*)expected_file_buffer + producer.kMaxChunkSize,
                             expected_file_size_overhead, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    producer.Send("", expected_file_buffer, expected_file_size);
}

TEST(ProducerImpl, Send__SEND_chunk2_data_io_error) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size_overhead = 1543;
    size_t                  expected_file_size = producer.kMaxChunkSize + expected_file_size_overhead;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;
    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkResponse))
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.Send("", nullptr, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, hidra2::PRODUCER_ERROR__SENDING_CHUNK_FAILED);
    ASSERT_THAT(status, hidra2::PRODUCER_STATUS__CONNECTED);
}

TEST(ProducerImpl, Send__receive_chunk2_meta_data_check) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size_overhead = 1543;
    size_t                  expected_file_size = producer.kMaxChunkSize + expected_file_size_overhead;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;
    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkResponse))
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(expected_fd, _/*addr*/, sizeof(hidra2::SendDataChunkResponse), Gt(0), _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    producer.Send("", nullptr, expected_file_size);
}

TEST(ProducerImpl, Send__receive_chunk2_meta_io_error) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size_overhead = 1543;
    size_t                  expected_file_size = producer.kMaxChunkSize + expected_file_size_overhead;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkResponse))
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(expected_fd, _/*addr*/, sizeof(hidra2::SendDataChunkResponse), Gt(0), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<4>(hidra2::IOError::BAD_FILE_NUMBER),
            Return(-1)
        ));

    hidra2::ProducerError error = producer.Send("", nullptr, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, hidra2::PRODUCER_ERROR__RECEIVING_SERVER_RESPONSE_FAILED);
    ASSERT_THAT(status, hidra2::PRODUCER_STATUS__CONNECTED);
}

TEST(ProducerImpl, Send__receive_chunk2_meta_server_error) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size_overhead = 1543;
    size_t                  expected_file_size = producer.kMaxChunkSize + expected_file_size_overhead;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkResponse))
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(expected_fd, _/*addr*/, sizeof(hidra2::SendDataChunkResponse), Gt(0), _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__INTERNAL_SERVER_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkResponse))
        ));

    hidra2::ProducerError error = producer.Send("", nullptr, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, hidra2::PRODUCER_ERROR__SERVER_REPORTED_AN_ERROR);
    ASSERT_THAT(status, hidra2::PRODUCER_STATUS__CONNECTED);
}


TEST(ProducerImpl, Send) {
    InSequence sequence;

    hidra2::ProducerImpl    producer;
    int                     expected_fd = 83942;
    uint64_t                expected_request_id = 0;
    uint64_t                expected_file_ref_id = 1338;
    size_t                  expected_file_size_overhead = 1543;
    size_t                  expected_file_size = producer.kMaxChunkSize + expected_file_size_overhead;

    connect_to_receiver_DONE(producer, expected_fd);
    expected_request_id++;

    hidra2::MockIO mockIO;
    producer.__set_io(&mockIO);

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(-1)
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkResponse))
        ));

    expected_request_id++;

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkRequest))
        ));

    EXPECT_CALL(mockIO, Send(_, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOError::NO_ERROR),
            Return(producer.kMaxChunkSize)
        ));

    EXPECT_CALL(mockIO, ReceiveTimeout(_, _/*addr*/, _, _, _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 0),
            testing::SetArgPointee<4>(hidra2::IOError::NO_ERROR),
            Return(sizeof(hidra2::SendDataChunkResponse))
        ));

    hidra2::ProducerError error = producer.Send("", nullptr, expected_file_size);
    hidra2::ProducerStatus status = producer.GetStatus();

    ASSERT_THAT(error, hidra2::PRODUCER_ERROR__NO_ERROR);
    ASSERT_THAT(status, hidra2::PRODUCER_STATUS__CONNECTED);
}

}
