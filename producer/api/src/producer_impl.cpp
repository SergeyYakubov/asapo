#include <system_wrappers/system_io.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include "producer_impl.h"

const uint32_t hidra2::ProducerImpl::kVersion = 1;

hidra2::ProducerImpl::ProducerImpl() {
    __set_io(ProducerImpl::kDefaultIO);
}

uint64_t hidra2::ProducerImpl::get_version() const {
    return kVersion;
}

hidra2::ProducerStatus hidra2::ProducerImpl::get_status() const {
    return PRODUCER_STATUS__CONNECTED;
}

hidra2::ProducerError hidra2::ProducerImpl::initialize_socket_to_receiver_(const std::string &receiver_address) {
    client_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    sockaddr_in socket_address {};
    socket_address.sin_addr.s_addr = inet_addr(receiver_address.c_str());
    socket_address.sin_port = htons(8099);
    socket_address.sin_family = AF_INET;

    if(io->connect(client_fd_, (struct sockaddr *)&socket_address, sizeof(socket_address)) == -1) {
        perror("Connecting to server");
        return PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER;
    }

    return PRODUCER_ERROR__OK;
}

hidra2::ProducerError hidra2::ProducerImpl::connect_to_receiver(const std::string& receiver_address) {
    if(client_fd_ != -1) {
        return PRODUCER_ERROR__ALREADY_CONNECTED;
    }

    ProducerError error;
    error = initialize_socket_to_receiver_(receiver_address);
    if(error) {
        return error;
    }

    hidra2::HelloRequest helloRequest;
    helloRequest.op_code = OP_CODE__HELLO;
    helloRequest.request_id = request_id++;
    helloRequest.client_version = 1;
    helloRequest.os = OS_LINUX;
    helloRequest.is_x64 = true;

    io_utils->send_in_steps(client_fd_, &helloRequest, sizeof(helloRequest), 0);

    HelloResponse helloResponse;

    io_utils->recv_in_steps(client_fd_, &helloResponse, sizeof(helloResponse), 0);

    std::cout << "op_code: " << helloResponse.op_code << std::endl;
    std::cout << "request_id: " << helloResponse.request_id << std::endl;
    std::cout << "error_code: " << helloResponse.error_code << std::endl;
    std::cout << "server_version: " << helloResponse.server_version << std::endl;


    if(helloResponse.error_code == NET_ERR__NO_ERROR) {
        return PRODUCER_ERROR__OK;
    }

    std::cerr << "Fail to connect to server. NetErrorCode: " << helloResponse.error_code << std::endl;

    return PRODUCER_ERROR__OK;
}

hidra2::ProducerError hidra2::ProducerImpl::send(std::string filename, void *data, uint64_t file_size) {
    hidra2::PrepareSendDataRequest prepareSendDataRequest;
    prepareSendDataRequest.op_code = OP_CODE__PREPARE_SEND_DATA;
    prepareSendDataRequest.request_id = request_id++;
    prepareSendDataRequest.file_size = file_size;
    filename.copy((char*)&prepareSendDataRequest.filename, sizeof(prepareSendDataRequest.filename));


    std::cout << "Send file: " << filename << std::endl;

    io_utils->send_in_steps(client_fd_, &prepareSendDataRequest, sizeof(prepareSendDataRequest), 0);

    hidra2::PrepareSendDataResponse prepareSendDataResponse;

    io_utils->recv_in_steps(client_fd_, &prepareSendDataResponse, sizeof(prepareSendDataResponse), 0);

    if(prepareSendDataResponse.error_code) {
        std::cerr << "Server rejected metadata. NetErrorCode: " << prepareSendDataResponse.error_code << std::endl;
        return PRODUCER_ERROR__SERVER_REPORTED_AN_ERROR;
    }
    std::cout << "op_code: " << prepareSendDataResponse.op_code << std::endl;
    std::cout << "request_id: " << prepareSendDataResponse.request_id << std::endl;
    std::cout << "error_code: " << prepareSendDataResponse.error_code << std::endl;
    std::cout << "file_reference_id: " << prepareSendDataResponse.file_reference_id << std::endl;


    NetworkErrorCode network_error = NET_ERR__NO_ERROR;
    size_t already_send = 0;
    uint64_t max_chunk_size = static_cast<uint64_t>(1024*1024*1024*2);

    while(!network_error && already_send < file_size) {
        size_t need_to_send = max_chunk_size;

        if(double(file_size) - already_send - max_chunk_size < 0) {
            need_to_send = file_size - already_send;
        }

        hidra2::SendDataChunkRequest sendDataChunkRequest;
        sendDataChunkRequest.op_code = OP_CODE__SEND_DATA_CHUNK;
        sendDataChunkRequest.request_id = request_id++;
        sendDataChunkRequest.start_byte = already_send;
        sendDataChunkRequest.chunk_size = need_to_send;
        sendDataChunkRequest.file_reference_id = prepareSendDataResponse.file_reference_id;

        if(io_utils->send_in_steps(client_fd_, &sendDataChunkRequest, sizeof(sendDataChunkRequest), 0) == -1) {
            std::cerr << "Fail to send chunk metadata. errno: " << errno << std::endl;
            return PRODUCER_ERROR__SENDING_CHUNK_FAILED;
        }

        if(io_utils->send_in_steps(client_fd_, data + already_send, need_to_send, 0) == -1) {
            std::cerr << "Fail to send chunk data. errno: " << errno << std::endl;
            return PRODUCER_ERROR__SENDING_CHUNK_FAILED;
        }

        already_send += need_to_send;

        hidra2::SendDataChunkResponse sendDataChunkResponse;

        if(io_utils->recv_in_steps(client_fd_, &sendDataChunkResponse, sizeof(sendDataChunkResponse), 0) == -1) {
            std::cout << "Failed to receive servers response. errno: " << errno << std::endl;
            return PRODUCER_ERROR__RECEIVING_SERVER_RESPONSE_FAILED;
        }

        if(sendDataChunkResponse.error_code) {
            std::cout << "Server reported an error. NetErrorCode: " << sendDataChunkResponse.error_code;
            return PRODUCER_ERROR__SERVER_REPORTED_AN_ERROR;
        }
    }

    return PRODUCER_ERROR__OK;
}

