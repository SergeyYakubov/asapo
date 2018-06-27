#include <iostream>
#include <cstring>

#include "producer_impl.h"
#include "producer_logger.h"
#include "io/io_factory.h"
#include "producer/producer_error.h"

namespace  asapo {

const size_t ProducerImpl::kMaxChunkSize = size_t(1024) * size_t(1024) * size_t(1024) * size_t(2); //2GiByte
const size_t ProducerImpl::kDiscoveryServiceUpdateFrequencyMs = 10000; // 10s


ProducerImpl::ProducerImpl(std::string endpoint, uint8_t n_processing_threads, asapo::RequestHandlerType type):
    log__{GetDefaultProducerLogger()} {
    switch (type) {
    case RequestHandlerType::kTcp:
        discovery_service_.reset(new ReceiverDiscoveryService{endpoint, ProducerImpl::kDiscoveryServiceUpdateFrequencyMs});
        request_handler_factory_.reset(new RequestHandlerFactory{discovery_service_.get()});
        break;
    case RequestHandlerType::kFilesystem:
        request_handler_factory_.reset(nullptr);
        request_handler_factory_.reset(new RequestHandlerFactory{endpoint});

    }
    request_pool__.reset(new RequestPool{n_processing_threads, request_handler_factory_.get()});
}

GenericRequestHeader ProducerImpl::GenerateNextSendRequest(uint64_t file_id, size_t file_size, std::string file_name) {
    GenericRequestHeader request{kOpcodeTransferData, file_id, file_size, std::move(file_name)};
    return request;
}

Error CheckProducerRequest(size_t file_size, size_t filename_size) {
    if (file_size > ProducerImpl::kMaxChunkSize) {
        return ProducerErrorTemplates::kFileTooLarge.Generate();
    }

    if (filename_size > kMaxMessageSize) {
        return ProducerErrorTemplates::kFileNameTooLong.Generate();
    }

    return nullptr;
}


Error ProducerImpl::Send(uint64_t file_id, const void* data, size_t file_size, std::string file_name,
                         RequestCallback callback) {

    auto err = CheckProducerRequest(file_size, file_name.size());
    if (err) {
        log__->Error("error checking request - " + err->Explain());
        return err;
    }


    auto request_header = GenerateNextSendRequest(file_id, file_size, std::move(file_name));


    return request_pool__->AddRequest(std::unique_ptr<Request> {new Request{beamtime_id_, request_header, data, callback}});
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

Error ProducerImpl::SetBeamtimeId(std::string beamtime_id) {

    if (!beamtime_id_.empty()) {
        log__->Error("beamtime_id already set");
        return ProducerErrorTemplates::kBeamtimeAlreadySet.Generate();
    }

    if (beamtime_id.size() > kMaxMessageSize) {
        log__->Error("beamtime_id is too long - " + beamtime_id);
        return ProducerErrorTemplates::kBeamtimeIdTooLong.Generate();
    }

    beamtime_id_ = std::move(beamtime_id);
    return nullptr;
}

}