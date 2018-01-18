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

uint64_t hidra2::ProducerImpl::GetVersion() const {
    return kVersion;
}

hidra2::ProducerStatus hidra2::ProducerImpl::GetStatus() const {
    return status_;
}

hidra2::ProducerError hidra2::ProducerImpl::initialize_socket_to_receiver_(const std::string& receiver_address) {
    IOError err;
    FileDescriptor fd = io->CreateAndConnectIPTCPSocket(receiver_address, &err);

    if(err != IOError::NO_ERROR) {
        if(err == IOError::INVALID_ADDRESS_FORMAT) {
            return PRODUCER_ERROR__INVALID_ADDRESS_FORMAT;
        }
        return PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER;
    }

    client_fd_ = fd;
    return PRODUCER_ERROR__NO_ERROR;
}

hidra2::ProducerError hidra2::ProducerImpl::ConnectToReceiver(const std::string& receiver_address) {
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

    status_ = PRODUCER_STATUS__CONNECTED;
    return PRODUCER_ERROR__NO_ERROR;
}

hidra2::ProducerError hidra2::ProducerImpl::Send(uint64_t file_id, void* data, uint64_t file_size) {
    if(status_ != PRODUCER_STATUS__CONNECTED) {
        return PRODUCER_ERROR__CONNECTION_NOT_READY;
    }
    status_ = PRODUCER_STATUS__SENDING;

    SendDataRequest sendDataRequest;
    sendDataRequest.op_code = OP_CODE__SEND_DATA;
    sendDataRequest.request_id = request_id++;
    sendDataRequest.file_id = file_id;
    sendDataRequest.file_size = file_size;

    IOError io_error;
    io->Send(client_fd_, &sendDataRequest, sizeof(sendDataRequest), &io_error);
    if(io_error != IOError::NO_ERROR) {
        std::cerr << "hidra2::ProducerImpl::send/send error" << std::endl;
        status_ = PRODUCER_STATUS__CONNECTED;
        return PRODUCER_ERROR__UNEXPECTED_IO_ERROR;
    }


    io->Send(client_fd_, data, file_size, &io_error);
    if(io_error != IOError::NO_ERROR) {
        status_ = PRODUCER_STATUS__CONNECTED;
        return PRODUCER_ERROR__UNEXPECTED_IO_ERROR;
    }

    SendDataResponse sendDataResponse;
    io->Receive(client_fd_, &sendDataResponse, sizeof(sendDataResponse), &io_error);
    if(io_error != IOError::NO_ERROR) {
        std::cerr << "hidra2::ProducerImpl::send/receive_timeout error" << std::endl;
        status_ = PRODUCER_STATUS__CONNECTED;
        return PRODUCER_ERROR__UNEXPECTED_IO_ERROR;
    }

    if(sendDataResponse.error_code) {
        std::cerr << "Server reported an error. NetErrorCode: " << sendDataResponse.error_code << std::endl;
        status_ = PRODUCER_STATUS__CONNECTED;
        return PRODUCER_ERROR__UNEXPECTED_IO_ERROR;
    }

    status_ = PRODUCER_STATUS__CONNECTED;
    return PRODUCER_ERROR__NO_ERROR;
}

