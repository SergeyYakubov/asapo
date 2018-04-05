#include <iostream>
#include <cstring>

#include "producer_impl.h"
#include "system/io_factory.h"

namespace  hidra2 {

const uint32_t ProducerImpl::kVersion = 1;
const size_t ProducerImpl::kMaxChunkSize = size_t(1024) * size_t(1024) * size_t(1024) * size_t(2); //2GiByte

ProducerImpl::ProducerImpl(): io__{GenerateDefaultIO()} {
}

uint64_t ProducerImpl::GetVersion() const {
    return kVersion;
}

ProducerStatus ProducerImpl::GetStatus() const {
    return status_;
}

Error ProducerImpl::InitializeSocketToReceiver(const std::string& receiver_address) {
    Error err;
    FileDescriptor fd = io__->CreateAndConnectIPTCPSocket(receiver_address, &err);
    if(err != nullptr) {
        return err;
    }

    client_fd_ = fd;
    return nullptr;
}

Error ProducerImpl::ConnectToReceiver(const std::string& receiver_address) {
    if(status_ != ProducerStatus::kDisconnected) {
        return ProducerErrorTemplates::kAlreadyConnected.Generate();
    }

    auto error = InitializeSocketToReceiver(receiver_address);
    if(error) {
        status_ = ProducerStatus::kDisconnected;
        return error;
    }

    status_ = ProducerStatus::kConnected;
    return nullptr;
}

GenericNetworkRequestHeader ProducerImpl::GenerateNextSendRequest(uint64_t file_id, size_t file_size) {
    GenericNetworkRequestHeader request;
    request.op_code = kNetOpcodeSendData;
    request.request_id = request_id_++;
    request.data_id = file_id;
    request.data_size = file_size;
    return request;
}

Error ProducerImpl::SendHeaderAndData(const GenericNetworkRequestHeader& header, const void* data, size_t file_size) {
    Error io_error;
    io__->Send(client_fd_, &header, sizeof(header), &io_error);
    if(io_error) {
        std::cerr << "ProducerImpl::Send/DataRequest error" << io_error << std::endl;
        return io_error;
    }

    io__->Send(client_fd_, data, file_size, &io_error);
    if(io_error) {
        std::cerr << "ProducerImpl::Send/data error" << io_error << std::endl;
        return io_error;
    }

    return nullptr;
}

Error ProducerImpl::ReceiveResponce() {
    Error err;
    SendDataResponse sendDataResponse;
    io__->Receive(client_fd_, &sendDataResponse, sizeof(sendDataResponse), &err);
    if(err != nullptr) {
        std::cerr << "ProducerImpl::Receive error: " << err << std::endl;
        return err;
    }

    if(sendDataResponse.error_code) {
        if(sendDataResponse.error_code == kNetErrorFileIdAlreadyInUse) {
            return ProducerErrorTemplates::kFileIdAlreadyInUse.Generate();
        }
        std::cerr << "Server reported an error. NetErrorCode: " << int(sendDataResponse.error_code) << std::endl;
        return ProducerErrorTemplates::kUnknownServerError.Generate();
    }
    return nullptr;
}


Error ProducerImpl::Send(uint64_t file_id, const void* data, size_t file_size) {
    if(status_ != ProducerStatus::kConnected) {
        return ProducerErrorTemplates::kConnectionNotReady.Generate();
    }
    if(file_size > kMaxChunkSize) {
        return ProducerErrorTemplates::kFileTooLarge.Generate();
    }

    auto send_data_request = GenerateNextSendRequest(file_id, file_size);

    auto  error = SendHeaderAndData(send_data_request, data, file_size);
    if(error) {
        return error;
    }

    return ReceiveResponce();
}

}