#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <netinet/in.h>
#include "../src/producer_impl.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Mock;
using ::testing::InSequence;

class MockIO : public hidra2::IO {
  public:
    MOCK_METHOD4(create_socket,
                 hidra2::FileDescriptor(hidra2::AddressFamilies address_family, hidra2::SocketTypes socket_type,
                                        hidra2::SocketProtocols socket_protocol, hidra2::IOErrors* err));
    MOCK_METHOD3(listen,
                 void(hidra2::FileDescriptor socket_fd, int backlog, hidra2::IOErrors* err));
    MOCK_METHOD4(inet_bind,
                 void(hidra2::FileDescriptor socket_fd, const std::string& address, uint16_t port, hidra2::IOErrors* err));
    MOCK_METHOD2(inet_accept,
                 std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>>(hidra2::FileDescriptor socket_fd,
                         hidra2::IOErrors* err));
    MOCK_METHOD3(inet_connect,
                 void(hidra2::FileDescriptor socket_fd, const std::string& address, hidra2::IOErrors* err));
    MOCK_METHOD2(create_and_connect_ip_tcp_socket,
                 hidra2::FileDescriptor(const std::string& address, hidra2::IOErrors* err));
    MOCK_METHOD4(receive,
                 size_t(hidra2::FileDescriptor socket_fd, void* buf, size_t length, hidra2::IOErrors* err));
    MOCK_METHOD5(receive_timeout,
                 size_t(hidra2::FileDescriptor socket_fd, void* buf, size_t length, uint16_t timeout_in_sec, hidra2::IOErrors* err));
    MOCK_METHOD4(send,
                 size_t(hidra2::FileDescriptor socket_fd, const void* buf, size_t length, hidra2::IOErrors* err));
    MOCK_METHOD2(deprecated_open,
                 int(const char* __file, int __oflag));
    MOCK_METHOD1(deprecated_close,
                 int(int __fd));
    MOCK_METHOD3(deprecated_read,
                 ssize_t(int __fd, void* buf, size_t count));
    MOCK_METHOD3(deprecated_write,
                 ssize_t(int __fd, const void* __buf, size_t __n));
    MOCK_METHOD2(GetDataFromFile,
                 hidra2::FileData(const std::string& fname, hidra2::IOErrors* err));
    MOCK_METHOD2(FilesInFolder,
                 std::vector<hidra2::FileInfo>(const std::string& folder, hidra2::IOErrors* err));
};

hidra2::ProducerImpl    producer;
uint64_t                expected_request_id = 0;
int                     expected_fd         = 83942;

TEST(get_version, VersionAboveZero) {
    hidra2::ProducerImpl producer;
    EXPECT_GE(producer.get_version(), 0);
}

TEST(ProducerImpl, get_status__disconnected) {
    EXPECT_EQ(producer.get_status(), hidra2::PRODUCER_STATUS__DISCONNECTED);
}

TEST(ProducerImpl, send__connection_not_ready) {
    EXPECT_EQ(producer.send("", nullptr, 1), hidra2::PRODUCER_ERROR__CONNECTION_NOT_READY);
}


/**
 * connect_to_receiver
 */

MATCHER_P(M_CheckHelloRequest, request_id, "Checks if a valid HelloRequest was send") {
    return ((hidra2::HelloRequest*)arg)->op_code == hidra2::OP_CODE__HELLO
           && ((hidra2::HelloRequest*)arg)->request_id == request_id;
}

ACTION_P3(A_WriteHelloResponse, error_code, request_id, server_version) {
    ((hidra2::HelloResponse*)arg1)->op_code = hidra2::OP_CODE__HELLO;
    ((hidra2::HelloResponse*)arg1)->error_code = error_code;
    ((hidra2::HelloResponse*)arg1)->request_id = request_id;
    ((hidra2::HelloResponse*)arg1)->server_version = server_version;
}

TEST(ProducerImpl, connect_to_receiver__invalid__address_format) {
    MockIO mockIO;
    producer.__set_io(&mockIO);

    std::string expected_address = "127.0.0.1:9090";

    EXPECT_CALL(mockIO, create_and_connect_ip_tcp_socket(expected_address, _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(hidra2::IOErrors::INVALID_ADDRESS_FORMAT),
                Return(-1)
            ));

    EXPECT_EQ(producer.connect_to_receiver(expected_address), hidra2::PRODUCER_ERROR__INVALID_ADDRESS_FORMAT);
    EXPECT_EQ(producer.get_status(), hidra2::PRODUCER_STATUS__DISCONNECTED);

    Mock::VerifyAndClearExpectations(&mockIO);
}

TEST(ProducerImpl, connect_to_receiver__failed_unk) {
    MockIO mockIO;
    producer.__set_io(&mockIO);

    std::string expected_address = "127.0.0.1:9090";

    EXPECT_CALL(mockIO, create_and_connect_ip_tcp_socket(expected_address, _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(hidra2::IOErrors::UNKNOWN_ERROR),
                Return(-1)
            ));

    EXPECT_EQ(producer.connect_to_receiver(expected_address), hidra2::PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER);
    EXPECT_EQ(producer.get_status(), hidra2::PRODUCER_STATUS__DISCONNECTED);

    Mock::VerifyAndClearExpectations(&mockIO);
}

TEST(ProducerImpl, connect_to_receiver) {
    MockIO mockIO;
    producer.__set_io(&mockIO);

    InSequence sequence;

    std::string expected_address = "127.0.0.1:9090";

    EXPECT_CALL(mockIO, create_and_connect_ip_tcp_socket(expected_address, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<1>(hidra2::IOErrors::NO_ERROR),
            Return(expected_fd)
        ));

    EXPECT_CALL(mockIO, send(expected_fd, M_CheckHelloRequest(expected_request_id), sizeof(hidra2::HelloRequest), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrors::NO_ERROR),
            Return(sizeof(hidra2::HelloRequest))
        ));

    EXPECT_CALL(mockIO, receive_timeout(expected_fd, _/*addr*/, sizeof(hidra2::HelloResponse), Gt(0), _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WriteHelloResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, 1),
            testing::SetArgPointee<4>(hidra2::IOErrors::NO_ERROR),
            Return(sizeof(hidra2::HelloResponse))
        ));

    expected_request_id++;

    EXPECT_EQ(producer.connect_to_receiver(expected_address), hidra2::PRODUCER_ERROR__NO_ERROR);
    EXPECT_EQ(producer.get_status(), hidra2::PRODUCER_STATUS__CONNECTED);

    Mock::VerifyAndClearExpectations(&mockIO);
}

TEST(ProducerImpl, connect_to_receiver__already_connected) {
    EXPECT_EQ(producer.connect_to_receiver(""), hidra2::PRODUCER_ERROR__ALREADY_CONNECTED);
    EXPECT_EQ(producer.get_status(), hidra2::PRODUCER_STATUS__CONNECTED);
}

/**
 * send
 */

MATCHER_P3(M_CheckPrepareSendDataRequest, request_id, filename, file_size,
           "Checks if a valid PrepareSendDataRequest was send") {
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
           "Checks if a valid SendDataChunkRequest was send") {
    return ((hidra2::SendDataChunkRequest*)arg)->op_code == hidra2::OP_CODE__SEND_DATA_CHUNK
           && ((hidra2::SendDataChunkRequest*)arg)->request_id == request_id
           && ((hidra2::SendDataChunkRequest*)arg)->file_reference_id == file_reference_id
           && ((hidra2::SendDataChunkRequest*)arg)->chunk_size == chunk_size
           && ((hidra2::SendDataChunkRequest*)arg)->start_byte == start_byte;
}

ACTION_P2(A_WriteSendDataChunkResponse, error_code, request_id) {
    ((hidra2::SendDataChunkResponse*)arg1)->op_code = hidra2::OP_CODE__SEND_DATA_CHUNK;
    ((hidra2::SendDataChunkResponse*)arg1)->error_code = error_code;
    ((hidra2::SendDataChunkResponse*)arg1)->request_id = request_id;
}

TEST(ProducerImpl, send) {
    MockIO mockIO;
    producer.__set_io(&mockIO);

    InSequence sequence;

    std::string expected_filename       = "somefilename.bin";
    void*       expected_file_buffer    = (void*)0xC0FFEE;
    size_t      expected_file_size      = (size_t)1024 * (size_t)1024 * (size_t)1024 * (size_t)4 + (size_t)300;
    uint64_t    expected_file_ref_id    = 1337;


    /* PrepareSendDataRequest */
    EXPECT_CALL(mockIO, send(expected_fd, M_CheckPrepareSendDataRequest(expected_request_id, expected_filename,
                             expected_file_size), sizeof(hidra2::PrepareSendDataRequest), _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(hidra2::IOErrors::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataRequest))
        ));

    /* PrepareSendDataResponse */
    EXPECT_CALL(mockIO, receive_timeout(expected_fd, _/*addr*/, sizeof(hidra2::PrepareSendDataResponse), Gt(0), _))
    .Times(1)
    .WillOnce(
        DoAll(
            A_WritePrepareSendDataResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id, expected_file_ref_id),
            testing::SetArgPointee<4>(hidra2::IOErrors::NO_ERROR),
            Return(sizeof(hidra2::PrepareSendDataResponse))
        ));

    expected_request_id++;


    size_t already_send = 0;

    while(already_send < expected_file_size) {

        size_t need_to_send = producer.kMaxChunkSize;
        if(double(expected_file_size) - already_send - producer.kMaxChunkSize < 0) {
            need_to_send = expected_file_size - already_send;
        }

        /* SendDataChunkRequest */
        EXPECT_CALL(mockIO, send(expected_fd, M_CheckSendDataChunkRequest(expected_request_id, expected_file_ref_id,
                                 need_to_send, already_send), sizeof(hidra2::SendDataChunkRequest), _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(hidra2::IOErrors::NO_ERROR),
                Return(sizeof(hidra2::SendDataChunkRequest))
            ));

        EXPECT_CALL(mockIO, send(expected_fd, (uint8_t*)expected_file_buffer + already_send, need_to_send, _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(hidra2::IOErrors::NO_ERROR),
                Return(need_to_send)
            ));

        /* SendDataChunkResponse */
        EXPECT_CALL(mockIO, receive_timeout(expected_fd, _/*addr*/, sizeof(hidra2::SendDataChunkResponse), Gt(0), _))
        .Times(1)
        .WillOnce(
            DoAll(
                A_WriteSendDataChunkResponse(hidra2::NET_ERR__NO_ERROR, expected_request_id),
                testing::SetArgPointee<4>(hidra2::IOErrors::NO_ERROR),
                Return(sizeof(hidra2::SendDataChunkResponse))
            ));

        expected_request_id++;

        already_send += need_to_send;
    }


    EXPECT_EQ(producer.send(expected_filename, expected_file_buffer, expected_file_size), hidra2::PRODUCER_ERROR__NO_ERROR);

    Mock::VerifyAndClearExpectations(&mockIO);
}

}
