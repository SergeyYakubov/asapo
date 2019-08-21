#include <iostream>
#include <iostream>
#include <cstring>

#include "producer_impl.h"
#include "producer_logger.h"
#include "io/io_factory.h"
#include "producer/producer_error.h"
#include "producer_request_handler_factory.h"
#include "producer_request.h"
#include "common/data_structs.h"


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

GenericRequestHeader ProducerImpl::GenerateNextSendRequest(const EventHeader& event_header, uint64_t injest_mode) {
    GenericRequestHeader request{kOpcodeTransferData, event_header.file_id, event_header.file_size,
                                 event_header.user_metadata.size(), std::move(event_header.file_name)};
    if (event_header.subset_id != 0) {
        request.op_code = kOpcodeTransferSubsetData;
        request.custom_data[kPosDataSetId] = event_header.subset_id;
        request.custom_data[kPosDataSetSize] = event_header.subset_size;
    }
    request.custom_data[kPosInjestMode] = injest_mode;
    return request;
}

Error CheckInjestMode(uint64_t injest_mode) {
    if ((injest_mode & IngestModeFlags::kTransferData) &&
            (injest_mode & IngestModeFlags::kTransferMetaDataOnly)) {
        return ProducerErrorTemplates::kWrongInjestMode.Generate();
    }

    if (!(injest_mode & IngestModeFlags::kTransferData) &&
            !(injest_mode & IngestModeFlags::kTransferMetaDataOnly)) {
        return ProducerErrorTemplates::kWrongInjestMode.Generate();
    }

    return nullptr;
}

Error CheckProducerRequest(const EventHeader& event_header, uint64_t injest_mode) {
    if ((size_t)event_header.file_size > ProducerImpl::kMaxChunkSize) {
        return ProducerErrorTemplates::kFileTooLarge.Generate();
    }

    if (event_header.file_name.size() > kMaxMessageSize) {
        return ProducerErrorTemplates::kFileNameTooLong.Generate();
    }

    if (event_header.file_name.empty() ) {
        return ProducerErrorTemplates::kEmptyFileName.Generate();
    }

    if (event_header.subset_id > 0 && event_header.subset_size == 0) {
        return ProducerErrorTemplates::kErrorSubsetSize.Generate();
    }

    return CheckInjestMode(injest_mode);
}

Error ProducerImpl::Send(const EventHeader& event_header,
                         FileData data,
                         std::string full_path,
                         uint64_t injest_mode,
                         RequestCallback callback) {
    auto err = CheckProducerRequest(event_header, injest_mode);
    if (err) {
        log__->Error("error checking request - " + err->Explain());
        return err;
    }

    auto request_header = GenerateNextSendRequest(event_header, injest_mode);

    return request_pool__->AddRequest(std::unique_ptr<ProducerRequest> {new ProducerRequest{source_cred_string_, std::move(request_header),
                std::move(data), std::move(event_header.user_metadata), std::move(full_path), callback}
    });

}


Error ProducerImpl::SendData(const EventHeader& event_header, FileData data,
                             uint64_t injest_mode, RequestCallback callback) {
    return Send(std::move(event_header), std::move(data), "", injest_mode, callback);
}


Error ProducerImpl::SendFile(const EventHeader& event_header, std::string full_path, uint64_t injest_mode,
                             RequestCallback callback) {
    if (full_path.empty()) {
        return ProducerErrorTemplates::kEmptyFileName.Generate();
    }

    return Send(event_header, nullptr, std::move(full_path), injest_mode, callback);
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

Error ProducerImpl::SetCredentials(SourceCredentials source_cred) {

    if (!source_cred_string_.empty()) {
        log__->Error("credentials already set");
        return ProducerErrorTemplates::kCredentialsAlreadySet.Generate();
    }

    if (source_cred.stream.empty()) {
        source_cred.stream = SourceCredentials::kDefaultStream;
    }

    source_cred_string_ = source_cred.GetString();
    if (source_cred_string_.size()  + source_cred.user_token.size() > kMaxMessageSize) {
        log__->Error("credentials string is too long - " + source_cred_string_);
        source_cred_string_ = "";
        return ProducerErrorTemplates::kCredentialsTooLong.Generate();
    }

    return nullptr;
}

Error ProducerImpl::SendMetaData(const std::string& metadata, RequestCallback callback) {
    GenericRequestHeader request_header{kOpcodeTransferMetaData, 0, metadata.size(), 0, "beamtime_global.meta"};
    request_header.custom_data[kPosInjestMode] = asapo::IngestModeFlags::kTransferData;
    FileData data{new uint8_t[metadata.size()]};
    strncpy((char*)data.get(), metadata.c_str(), metadata.size());
    return request_pool__->AddRequest(std::unique_ptr<ProducerRequest> {new ProducerRequest{source_cred_string_, std::move(request_header),
                std::move(data), "", "", callback}
    });
}

}