#include <iostream>
#include <iostream>
#include <cstring>

#include "producer_impl.h"
#include "producer_logger.h"
#include "io/io_factory.h"
#include "producer/producer_error.h"
#include "producer_request_handler_factory.h"
#include "producer_request.h"

namespace  asapo {

const size_t ProducerImpl::kMaxChunkSize = size_t(1024) * size_t(1024) * size_t(1024) * size_t(2); //2GiByte
const size_t ProducerImpl::kDiscoveryServiceUpdateFrequencyMs = 10000; // 10s


ProducerImpl::ProducerImpl(std::string endpoint, uint8_t n_processing_threads, asapo::RequestHandlerType type):
    log__{GetDefaultProducerLogger()} {
    switch (type) {
    case RequestHandlerType::kTcp:
        discovery_service_.reset(new ReceiverDiscoveryService{endpoint, ProducerImpl::kDiscoveryServiceUpdateFrequencyMs});
        request_handler_factory_.reset(new ProducerRequestHandlerFactory{discovery_service_.get()});
        break;
    case RequestHandlerType::kFilesystem:
        request_handler_factory_.reset(new ProducerRequestHandlerFactory{endpoint});
        break;
    }
    request_pool__.reset(new RequestPool{n_processing_threads, request_handler_factory_.get(), log__});
}

GenericRequestHeader ProducerImpl::GenerateNextSendRequest(uint64_t file_id, uint64_t file_size,
        std::string file_name) {
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

Error ProducerImpl::Send(const EventHeader& event_header,
                         FileData data,
                         std::string full_path,
                         RequestCallback callback) {
    auto err = CheckProducerRequest((size_t)event_header.file_size, event_header.file_name.size());
    if (err) {
        log__->Error("error checking request - " + err->Explain());
        return err;
    }

    auto request_header = GenerateNextSendRequest(event_header.file_id, event_header.file_size, event_header.file_name);

    return request_pool__->AddRequest(std::unique_ptr<ProducerRequest> {new ProducerRequest{beamtime_id_, std::move(request_header),
                std::move(data), std::move(full_path), callback}
    });

}


Error ProducerImpl::SendData(const EventHeader& event_header, FileData data,
                             RequestCallback callback) {
    return Send(event_header, std::move(data), "", callback);
}


Error ProducerImpl::SendFile(const EventHeader& event_header, std::string full_path, RequestCallback callback) {
    return Send(event_header, nullptr, std::move(full_path), callback);
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

Error ProducerImpl::SendMetaData(const std::string& metadata, RequestCallback callback) {
    GenericRequestHeader request_header{kOpcodeTransferMetaData, 0, metadata.size(), beamtime_id_ + ".meta"};
    FileData data{new uint8_t[metadata.size()]};
    strncpy((char*)data.get(), metadata.c_str(),metadata.size());
    return request_pool__->AddRequest(std::unique_ptr<ProducerRequest> {new ProducerRequest{beamtime_id_, std::move(request_header),
                std::move(data), "", callback}
    });
}

}