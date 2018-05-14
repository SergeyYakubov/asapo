#include <iostream>
#include <cstring>

#include "producer_impl.h"
#include "producer_logger.h"
#include "io/io_factory.h"

namespace  asapo {

const uint32_t ProducerImpl::kVersion = 1;
const size_t ProducerImpl::kMaxChunkSize = size_t(1024) * size_t(1024) * size_t(1024) * size_t(2); //2GiByte

ProducerImpl::ProducerImpl(): io__{GenerateDefaultIO()},log__{GetDefaultProducerLogger()} {
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
        log__->Debug("cannot connect to receiver at " + receiver_address + " - " + err->Explain());
        return err;
    }

    receiver_uri_ = receiver_address;
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
    log__->Info("connected to receiver at " + receiver_address);
    return nullptr;
}

GenericNetworkRequestHeader ProducerImpl::GenerateNextSendRequest(uint64_t file_id, size_t file_size) {
    GenericNetworkRequestHeader request;
    request.op_code = kNetOpcodeSendData;
    request.data_id = file_id;
    request.data_size = file_size;
    return request;
}


Error ProducerImpl::ReceiveResponce() {
    Error err;
    SendDataResponse sendDataResponse;
    io__->Receive(client_fd_, &sendDataResponse, sizeof(sendDataResponse), &err);
    if(err != nullptr) {
//        std::cerr << "ProducerImpl::Receive error: " << err << std::endl;
        return err;
    }

    if(sendDataResponse.error_code) {
        if(sendDataResponse.error_code == kNetErrorFileIdAlreadyInUse) {
            return ProducerErrorTemplates::kFileIdAlreadyInUse.Generate();
        }
//        std::cerr << "Server reported an error. NetErrorCode: " << int(sendDataResponse.error_code) << std::endl;
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

    /*auto  error = SendHeaderAndData(send_data_request, data, file_size);
    if(error) {
        log__->Debug("error sending to " + receiver_uri_ + " - " + error->Explain());
        return error;
    }

    error =  ReceiveResponce();
    if(error) {
        log__->Debug("error receiving response from " + receiver_uri_ + " - " + error->Explain());
        return error;
    }

     */

    log__->Debug("succesfully sent data to " + receiver_uri_);

    return nullptr;
}

void ProducerImpl::SetLogLevel(LogLevel level) {
    log__->SetLogLevel(level);
}

void ProducerImpl::EnableLocalLog(bool enable) {
    log__->EnableLocalLog(enable);
}

void ProducerImpl::EnableRemoteLog(bool enable) {
    log__->EnableRemoteLog(enable);
}

}