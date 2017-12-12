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
    IOErrors err;
    FileDescriptor fd = io->create_and_connect_ip_tcp_socket(receiver_address, &err);

    if(err != IOErrors::NO_ERROR) {
        if(err == IOErrors::INVALID_ADDRESS_FORMAT) {
            return PRODUCER_ERROR__INVALID_ADDRESS_FORMAT;
        }
        return PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER;
    }

    client_fd_ = fd;
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

    IOErrors io_error;
    io->send(client_fd_, &helloRequest, sizeof(helloRequest), &io_error);
    if(io_error != IOErrors::NO_ERROR) {
        std::cerr << "hidra2::ProducerImpl::connect_to_receiver/send error" << std::endl;
        return PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER;
    }

    HelloResponse helloResponse;
    io->receive_timeout(client_fd_, &helloResponse, sizeof(helloResponse), 30, &io_error);
    if(io_error != IOErrors::NO_ERROR) {
        std::cerr << "hidra2::ProducerImpl::connect_to_receiver/receive_timeout error" << std::endl;
        return PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER;
    }

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

    IOErrors io_error;
    io->send(client_fd_, &prepareSendDataRequest, sizeof(prepareSendDataRequest), &io_error);
    if(io_error != IOErrors::NO_ERROR) {
        std::cerr << "hidra2::ProducerImpl::send/send error" << std::endl;
        return PRODUCER_ERROR__SENDING_SERVER_REQUEST_FAILED;
    }

    hidra2::PrepareSendDataResponse prepareSendDataResponse;

    io->receive_timeout(client_fd_, &prepareSendDataResponse, sizeof(prepareSendDataResponse), 30, &io_error);
    if(io_error != IOErrors::NO_ERROR) {
        std::cerr << "hidra2::ProducerImpl::send/receive_timeout error" << std::endl;
        return PRODUCER_ERROR__RECEIVING_SERVER_RESPONSE_FAILED;
    }

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
    uint64_t max_chunk_size = (uint64_t)1024*(uint64_t)1024*(uint64_t)1024*(uint64_t)2;

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

        io->send(client_fd_, &sendDataChunkRequest, sizeof(sendDataChunkRequest), &io_error);
        if(io_error != IOErrors::NO_ERROR) {
            std::cerr << "hidra2::ProducerImpl::send/send2 error" << std::endl;
            return PRODUCER_ERROR__SENDING_CHUNK_FAILED;
        }

        io->send(client_fd_, (uint8_t*)data + already_send, need_to_send, &io_error);
        if(io_error != IOErrors::NO_ERROR) {
            std::cerr << "hidra2::ProducerImpl::send/send3 error" << std::endl;
            return PRODUCER_ERROR__SENDING_CHUNK_FAILED;
        }

        already_send += need_to_send;

        hidra2::SendDataChunkResponse sendDataChunkResponse;
        io->receive_timeout(client_fd_, &sendDataChunkResponse, sizeof(sendDataChunkResponse), 30, &io_error);
        if(io_error != IOErrors::NO_ERROR) {
            std::cerr << "hidra2::ProducerImpl::send/receive_timeout2 error" << std::endl;
            return PRODUCER_ERROR__RECEIVING_SERVER_RESPONSE_FAILED;
        }

        if(sendDataChunkResponse.error_code) {
            std::cout << "Server reported an error. NetErrorCode: " << sendDataChunkResponse.error_code;
            return PRODUCER_ERROR__SERVER_REPORTED_AN_ERROR;
        }
    }

    return PRODUCER_ERROR__OK;
}

