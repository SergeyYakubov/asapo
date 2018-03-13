#include <iostream>
#include <cstring>
#include "producer_impl.h"

const uint32_t hidra2::ProducerImpl::kVersion = 1;
const size_t hidra2::ProducerImpl::kMaxChunkSize = size_t(1024) * size_t(1024) * size_t(1024) * size_t(2); //2GiByte

hidra2::ProducerError hidra2::ProducerImpl::NetworkErrorToProducerError(hidra2::NetworkErrorCode networkError) {
    switch(networkError) {
    case NET_ERR__NO_ERROR:
        return ProducerError::kNoError;
    case NET_ERR__FILEID_ALREADY_IN_USE:
        return ProducerError::kFileIdAlreadyInUse;
    default:
        return ProducerError::kUnknownServerError;
    }
}

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
    Error err;
    FileDescriptor fd = io->CreateAndConnectIPTCPSocket(receiver_address, &err);

    if(err != nullptr) {
        if(IOErrorTemplates::kInvalidAddressFormat == err) {
            return ProducerError::kInvalidAddressFormat;
        }
        if(IOErrorTemplates::kConnectionRefused == err) {
            return ProducerError::kConnectionRefused;
        }
        return ProducerError::kUnknownError;
    }

    client_fd_ = fd;
    return ProducerError::kNoError;
}

hidra2::ProducerError hidra2::ProducerImpl::ConnectToReceiver(const std::string& receiver_address) {
    if(client_fd_ != -1 && status_ != ProducerStatus::kDisconnected) {
        return ProducerError::kAlreadyConnected;
    }

    ProducerError error;
    error = initialize_socket_to_receiver_(receiver_address);
    if(error != ProducerError::kNoError) {
        status_ = ProducerStatus::kDisconnected;
        return error;
    }

    status_ = ProducerStatus::kConnected;
    return ProducerError::kNoError;
}

hidra2::ProducerError hidra2::ProducerImpl::Send(uint64_t file_id, const void* data, size_t file_size) {
    if(status_ != ProducerStatus::kConnected) {
        return ProducerError::kConnectionNotReady;
    }
    if(file_size > kMaxChunkSize) {
        return ProducerError::kFileTooLarge;
    }

    SendDataRequest sendDataRequest;
    sendDataRequest.op_code = kNetOpcodeSendData;
    sendDataRequest.request_id = request_id++;
    sendDataRequest.file_id = file_id;
    sendDataRequest.file_size = file_size;

    Error io_error;
    io->Send(client_fd_, &sendDataRequest, sizeof(sendDataRequest), &io_error);
    if(io_error != nullptr) {
        std::cerr << "hidra2::ProducerImpl::Send/sendDataRequest" << std::endl;
        status_ = ProducerStatus::kConnected;
        return ProducerError::kUnexpectedIOError;
    }

    io->Send(client_fd_, data, file_size, &io_error);
    if(io_error != nullptr) {
        std::cerr << "hidra2::ProducerImpl::Send/sendData" << std::endl;
        status_ = ProducerStatus::kConnected;
        return ProducerError::kUnexpectedIOError;
    }

    SendDataResponse sendDataResponse;
    io->Receive(client_fd_, &sendDataResponse, sizeof(sendDataResponse), &io_error);
    if(io_error != nullptr) {
        std::cerr << "hidra2::ProducerImpl::send/receive_timeout error" << std::endl;
        status_ = ProducerStatus::kConnected;
        return ProducerError::kUnexpectedIOError;
    }

    if(sendDataResponse.error_code) {
        status_ = ProducerStatus::kConnected;
        if(sendDataResponse.error_code == NET_ERR__FILEID_ALREADY_IN_USE) {
            return hidra2::ProducerError::kFileIdAlreadyInUse;
        }
        std::cerr << "Server reported an error. NetErrorCode: " << sendDataResponse.error_code << std::endl;
        return ProducerError::kUnknownServerError;
    }

    status_ = ProducerStatus::kConnected;
    return ProducerError::kNoError;
}
