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

const size_t ProducerImpl::kDiscoveryServiceUpdateFrequencyMs = 10000; // 10s
const std::string ProducerImpl::kFinishSubStreamKeyword = "asapo_finish_substream";
const std::string ProducerImpl::kNoNextSubStreamKeyword = "asapo_no_next";


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

GenericRequestHeader ProducerImpl::GenerateNextSendRequest(const EventHeader& event_header, std::string substream,
        uint64_t ingest_mode) {
    GenericRequestHeader request{kOpcodeTransferData, event_header.file_id, event_header.file_size,
                                 event_header.user_metadata.size(), event_header.file_name, substream};
    if (event_header.subset_id != 0) {
        request.op_code = kOpcodeTransferSubsetData;
        request.custom_data[kPosDataSetId] = event_header.subset_id;
        request.custom_data[kPosDataSetSize] = event_header.subset_size;
    }
    request.custom_data[kPosIngestMode] = ingest_mode;
    return request;
}

Error CheckIngestMode(uint64_t ingest_mode) {
    if ((ingest_mode & IngestModeFlags::kTransferData) &&
            (ingest_mode & IngestModeFlags::kTransferMetaDataOnly)) {
        return ProducerErrorTemplates::kWrongInput.Generate("wrong ingest mode");
    }

    if (!(ingest_mode & IngestModeFlags::kTransferData) &&
            !(ingest_mode & IngestModeFlags::kTransferMetaDataOnly)) {
        return ProducerErrorTemplates::kWrongInput.Generate("wrong ingest mode");
    }

    return nullptr;
}

Error CheckProducerRequest(const EventHeader& event_header, uint64_t ingest_mode) {
    if (event_header.file_name.size() > kMaxMessageSize) {
        return ProducerErrorTemplates::kWrongInput.Generate("too long filename");
    }

    if (event_header.file_name.empty() ) {
        return ProducerErrorTemplates::kWrongInput.Generate("empty filename");
    }

    if (event_header.subset_id > 0 && event_header.subset_size == 0) {
        return ProducerErrorTemplates::kWrongInput.Generate("subset dimensions");
    }

    return CheckIngestMode(ingest_mode);
}

Error ProducerImpl::Send(const EventHeader& event_header,
                         std::string substream,
                         FileData data,
                         std::string full_path,
                         uint64_t ingest_mode,
                         RequestCallback callback,
                         bool manage_data_memory) {
    auto err = CheckProducerRequest(event_header, ingest_mode);
    if (err) {
        log__->Error("error checking request - " + err->Explain());
        return err;
    }

    auto request_header = GenerateNextSendRequest(event_header, std::move(substream), ingest_mode);

    return request_pool__->AddRequest(std::unique_ptr<ProducerRequest> {new ProducerRequest{source_cred_string_, std::move(request_header),
                std::move(data), std::move(event_header.user_metadata), std::move(full_path), callback, manage_data_memory}
    });

}

bool WandTransferData(uint64_t ingest_mode) {
    return ingest_mode & IngestModeFlags::kTransferData;
}

Error CheckData(uint64_t ingest_mode, const EventHeader& event_header, const FileData* data) {
    if (WandTransferData(ingest_mode)) {
        if (*data == nullptr) {
            return ProducerErrorTemplates::kWrongInput.Generate("need data for this ingest mode");
        }
        if (event_header.file_size == 0) {
            return ProducerErrorTemplates::kWrongInput.Generate("zero data size");
        }
    }
    return nullptr;
}

Error ProducerImpl::SendData(const EventHeader& event_header, FileData data,
                             uint64_t ingest_mode, RequestCallback callback) {
    return SendData(event_header, kDefaultSubstream, std::move(data), ingest_mode, callback);
}

Error ProducerImpl::SendData(const EventHeader& event_header,
                             std::string substream,
                             FileData data,
                             uint64_t ingest_mode,
                             RequestCallback callback) {
    if (auto err = CheckData(ingest_mode, event_header, &data)) {
        return err;
    }
    return Send(event_header, std::move(substream), std::move(data), "", ingest_mode, callback, true);

}

Error ProducerImpl::SendSubstreamFinishedFlag(std::string substream, uint64_t last_id, std::string next_substream,
                                              RequestCallback callback) {
    EventHeader event_header;
    event_header.file_name = kFinishSubStreamKeyword;
    event_header.file_size = 0;
    event_header.file_id = last_id + 1;
    if (next_substream.empty()) {
        next_substream = kNoNextSubStreamKeyword;
    }
    event_header.user_metadata =  std::string("{\"next_substream\":") + "\"" + next_substream + "\"}";
    return Send(event_header, std::move(substream), nullptr, "", IngestModeFlags::kTransferMetaDataOnly, callback, true);
}

Error ProducerImpl::SendFile(const EventHeader& event_header, std::string full_path, uint64_t ingest_mode,
                             RequestCallback callback) {
    return SendFile(event_header, kDefaultSubstream, std::move(full_path), ingest_mode, callback);
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
        return ProducerErrorTemplates::kWrongInput.Generate("credentials already set");
    }

    if (source_cred.stream.empty()) {
        source_cred.stream = SourceCredentials::kDefaultStream;
    }

    source_cred_string_ = source_cred.GetString();
    if (source_cred_string_.size()  + source_cred.user_token.size() > kMaxMessageSize) {
        log__->Error("credentials string is too long - " + source_cred_string_);
        source_cred_string_ = "";
        return ProducerErrorTemplates::kWrongInput.Generate("credentials string is too long");
    }

    return nullptr;
}

Error ProducerImpl::SendMetaData(const std::string& metadata, RequestCallback callback) {
    GenericRequestHeader request_header{kOpcodeTransferMetaData, 0, metadata.size(), 0, "beamtime_global.meta"};
    request_header.custom_data[kPosIngestMode] = asapo::IngestModeFlags::kTransferData;
    FileData data{new uint8_t[metadata.size()]};
    strncpy((char*)data.get(), metadata.c_str(), metadata.size());
    return request_pool__->AddRequest(std::unique_ptr<ProducerRequest> {new ProducerRequest{source_cred_string_, std::move(request_header),
                std::move(data), "", "", callback, true}
    });
}

Error ProducerImpl::SendData__(const EventHeader& event_header,
                               std::string substream,
                               void* data,
                               uint64_t ingest_mode,
                               RequestCallback callback) {
    FileData data_wrapped = FileData{(uint8_t*)data};

    if (auto err = CheckData(ingest_mode, event_header, &data_wrapped)) {
        return err;
    }

    return Send(std::move(event_header), std::move(substream), std::move(data_wrapped), "", ingest_mode, callback, false);
}

Error ProducerImpl::SendData__(const EventHeader& event_header,
                               void* data,
                               uint64_t ingest_mode,
                               RequestCallback callback) {
    return SendData__(event_header, kDefaultSubstream, data, ingest_mode, callback);
}

uint64_t  ProducerImpl::GetRequestsQueueSize() {
    return request_pool__->NRequestsInPool();
};

Error ProducerImpl::WaitRequestsFinished(uint64_t timeout_ms) {
    if (request_pool__->WaitRequestsFinished(timeout_ms) != nullptr) {
        return ProducerErrorTemplates::kTimeout.Generate("waiting to finish processing requests");
    } else {
        return nullptr;
    }
}

void ProducerImpl::StopThreads__() {
    request_pool__->StopThreads();
}
Error ProducerImpl::SendFile(const EventHeader& event_header,
                             std::string substream,
                             std::string full_path,
                             uint64_t ingest_mode,
                             RequestCallback callback) {
    if (full_path.empty()) {
        return ProducerErrorTemplates::kWrongInput.Generate("empty filename");
    }

    return Send(event_header, std::move(substream), nullptr, std::move(full_path), ingest_mode, callback, true);

}

}