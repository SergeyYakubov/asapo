#include <iostream>
#include <cstring>

#include "producer_impl.h"
#include "producer_logger.h"
#include "io/io_factory.h"

namespace  asapo {

const size_t ProducerImpl::kMaxChunkSize = size_t(1024) * size_t(1024) * size_t(1024) * size_t(2); //2GiByte
const size_t ProducerImpl::kDiscoveryServiceUpdateFrequencyMs = 10000; // 10s


ProducerImpl::ProducerImpl(std::string endpoint, uint8_t n_processing_threads): log__{GetDefaultProducerLogger()} {
    discovery_service_.reset(new ReceiverDiscoveryService{endpoint, ProducerImpl::kDiscoveryServiceUpdateFrequencyMs});
    request_handler_factory_.reset(new RequestHandlerFactory{RequestHandlerType::kTcp,discovery_service_.get()});
    request_pool__.reset(new RequestPool{n_processing_threads, request_handler_factory_.get()});
}

GenericNetworkRequestHeader ProducerImpl::GenerateNextSendRequest(uint64_t file_id, size_t file_size) {
    GenericNetworkRequestHeader request;
    request.op_code = kNetOpcodeSendData;
    request.data_id = file_id;
    request.data_size = file_size;
    return request;
}

Error CheckProducerRequest(const GenericNetworkRequestHeader header) {
    if (header.data_size > ProducerImpl::kMaxChunkSize) {
        return ProducerErrorTemplates::kFileTooLarge.Generate();
    }

    return nullptr;
}


Error ProducerImpl::Send(uint64_t file_id, const void* data, size_t file_size, RequestCallback callback) {
    auto request_header = GenerateNextSendRequest(file_id, file_size);

    auto err = CheckProducerRequest(request_header);
    if (err) {
        return err;
    }

    return request_pool__->AddRequest(std::unique_ptr<Request> {new Request{request_header, data, callback}});
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