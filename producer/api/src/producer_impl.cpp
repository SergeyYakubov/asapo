#include <system_wrappers/system_io.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <sys/sendfile.h>
#include <fcntl.h>
#include "producer_impl.h"

const uint32_t hidra2::ProducerImpl::kVersion = 1;
const size_t hidra2::ProducerImpl::kMaxChunkSize = (size_t)1024 * (size_t)1024 * (size_t)1024 * (size_t)2; //2GiByte

hidra2::ProducerImpl::ProducerImpl() {
    __set_io(ProducerImpl::kDefaultIO);
}

uint64_t hidra2::ProducerImpl::get_version() const {
    return kVersion;
}

hidra2::ProducerStatus hidra2::ProducerImpl::get_status() const {
    return status_;
}

hidra2::ProducerError hidra2::ProducerImpl::initialize_socket_to_receiver_(const std::string& receiver_address) {
    IOError err;
    FileDescriptor fd = io->create_and_connect_ip_tcp_socket(receiver_address, &err);

    if(err != IOError::NO_ERROR) {
        if(err == IOError::INVALID_ADDRESS_FORMAT) {
            return PRODUCER_ERROR__INVALID_ADDRESS_FORMAT;
        }
        return PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER;
    }

    client_fd_ = fd;
    return PRODUCER_ERROR__NO_ERROR;
}

hidra2::ProducerError hidra2::ProducerImpl::connect_to_receiver(const std::string& receiver_address) {
    if(client_fd_ != -1 && status_ != PRODUCER_STATUS__DISCONNECTED && status_ != PRODUCER_STATUS__CONNECTING) {
        return PRODUCER_ERROR__ALREADY_CONNECTED;
    }

    status_ = PRODUCER_STATUS__CONNECTING;

    ProducerError error;
    error = initialize_socket_to_receiver_(receiver_address);
    if(error) {
        status_ = PRODUCER_STATUS__DISCONNECTED;
        return error;
    }

    hidra2::HelloRequest helloRequest;
    helloRequest.op_code = OP_CODE__HELLO;
    helloRequest.request_id = request_id++;
    helloRequest.client_version = 1;
    helloRequest.os = OS_LINUX;
    helloRequest.is_x64 = true;

    IOError io_error;
    io->send(client_fd_, &helloRequest, sizeof(helloRequest), &io_error);
    if(io_error != IOError::NO_ERROR) {
        std::cerr << "hidra2::ProducerImpl::connect_to_receiver/send error" << std::endl;
        status_ = PRODUCER_STATUS__DISCONNECTED;
        return PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER;
    }

    HelloResponse helloResponse;
    io->receive_timeout(client_fd_, &helloResponse, sizeof(helloResponse), 30, &io_error);
    if(io_error != IOError::NO_ERROR) {
        std::cerr << "hidra2::ProducerImpl::connect_to_receiver/receive_timeout error" << std::endl;
        status_ = PRODUCER_STATUS__DISCONNECTED;
        return PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER;
    }

    std::cout << "op_code: " << helloResponse.op_code << std::endl;
    std::cout << "request_id: " << helloResponse.request_id << std::endl;
    std::cout << "error_code: " << helloResponse.error_code << std::endl;
    std::cout << "server_version: " << helloResponse.server_version << std::endl;


    if(helloResponse.error_code != NET_ERR__NO_ERROR) {
        std::cerr << "Fail to connect to server. NetErrorCode: " << helloResponse.error_code << std::endl;

        status_ = PRODUCER_STATUS__DISCONNECTED;
        return PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER;
    }

    status_ = PRODUCER_STATUS__CONNECTED;
    return PRODUCER_ERROR__NO_ERROR;
}

hidra2::ProducerError hidra2::ProducerImpl::send(std::string filename, void* data, uint64_t file_size) {
    if(status_ != PRODUCER_STATUS__CONNECTED) {
        return PRODUCER_ERROR__CONNECTION_NOT_READY;
    }
    status_ = PRODUCER_STATUS__SENDING;

    hidra2::PrepareSendDataRequest prepareSendDataRequest;
    prepareSendDataRequest.op_code = OP_CODE__PREPARE_SEND_DATA;
    prepareSendDataRequest.request_id = request_id++;
    prepareSendDataRequest.file_size = file_size;
    filename.copy((char*)&prepareSendDataRequest.filename, sizeof(prepareSendDataRequest.filename));
    prepareSendDataRequest.filename[filename.length()] = '\0';

    std::cout << "Send file: " << filename << std::endl;

    IOError io_error;
    io->send(client_fd_, &prepareSendDataRequest, sizeof(prepareSendDataRequest), &io_error);
    if(io_error != IOError::NO_ERROR) {
        std::cerr << "hidra2::ProducerImpl::send/send error" << std::endl;
        status_ = PRODUCER_STATUS__CONNECTED;
        return PRODUCER_ERROR__SENDING_SERVER_REQUEST_FAILED;
    }

    hidra2::PrepareSendDataResponse prepareSendDataResponse;

    io->receive_timeout(client_fd_, &prepareSendDataResponse, sizeof(prepareSendDataResponse), 30, &io_error);
    if(io_error != IOError::NO_ERROR) {
        std::cerr << "hidra2::ProducerImpl::send/receive_timeout error" << std::endl;
        status_ = PRODUCER_STATUS__CONNECTED;
        return PRODUCER_ERROR__RECEIVING_SERVER_RESPONSE_FAILED;
    }

    if(prepareSendDataResponse.error_code) {
        std::cerr << "Server rejected metadata. NetErrorCode: " << prepareSendDataResponse.error_code << std::endl;
        status_ = PRODUCER_STATUS__CONNECTED;
        return PRODUCER_ERROR__SERVER_REPORTED_AN_ERROR;
    }
    std::cout << "op_code: " << prepareSendDataResponse.op_code << std::endl;
    std::cout << "request_id: " << prepareSendDataResponse.request_id << std::endl;
    std::cout << "error_code: " << prepareSendDataResponse.error_code << std::endl;
    std::cout << "file_reference_id: " << prepareSendDataResponse.file_reference_id << std::endl;

    size_t already_send = 0;

    while(already_send < file_size) {
        size_t need_to_send = kMaxChunkSize;

        if(double(file_size) - already_send - kMaxChunkSize < 0) {
            need_to_send = file_size - already_send;
        }

        hidra2::SendDataChunkRequest sendDataChunkRequest;
        sendDataChunkRequest.op_code = OP_CODE__SEND_DATA_CHUNK;
        sendDataChunkRequest.request_id = request_id++;
        sendDataChunkRequest.start_byte = already_send;
        sendDataChunkRequest.chunk_size = need_to_send;
        sendDataChunkRequest.file_reference_id = prepareSendDataResponse.file_reference_id;

        io->send(client_fd_, &sendDataChunkRequest, sizeof(sendDataChunkRequest), &io_error);
        if(io_error != IOError::NO_ERROR) {
            std::cerr << "hidra2::ProducerImpl::send/send2 error" << std::endl;
            status_ = PRODUCER_STATUS__CONNECTED;
            return PRODUCER_ERROR__SENDING_CHUNK_FAILED;
        }

        io->send(client_fd_, (uint8_t*)data + already_send, need_to_send, &io_error);
        if(io_error != IOError::NO_ERROR) {
            std::cerr << "hidra2::ProducerImpl::send/send3 error" << std::endl;
            status_ = PRODUCER_STATUS__CONNECTED;
            return PRODUCER_ERROR__SENDING_CHUNK_FAILED;
        }

        already_send += need_to_send;

        hidra2::SendDataChunkResponse sendDataChunkResponse;
        io->receive_timeout(client_fd_, &sendDataChunkResponse, sizeof(sendDataChunkResponse), 30, &io_error);
        if(io_error != IOError::NO_ERROR) {
            std::cerr << "hidra2::ProducerImpl::send/receive_timeout2 error" << std::endl;
            status_ = PRODUCER_STATUS__CONNECTED;
            return PRODUCER_ERROR__RECEIVING_SERVER_RESPONSE_FAILED;
        }

        if(sendDataChunkResponse.error_code) {
            std::cout << "Server reported an error. NetErrorCode: " << sendDataChunkResponse.error_code;
            status_ = PRODUCER_STATUS__CONNECTED;
            return PRODUCER_ERROR__SERVER_REPORTED_AN_ERROR;
        }
    }

    status_ = PRODUCER_STATUS__CONNECTED;
    return PRODUCER_ERROR__NO_ERROR;
}

