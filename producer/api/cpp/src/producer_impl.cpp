#include <iostream>
#include <iostream>
#include <cstring>
#include <future>

#include "producer_impl.h"
#include "producer_logger.h"
#include "asapo/io/io_factory.h"
#include "asapo/producer/producer_error.h"
#include "producer_request_handler_factory.h"
#include "producer_request.h"
#include "asapo/common/data_structs.h"


namespace  asapo {

const size_t ProducerImpl::kDiscoveryServiceUpdateFrequencyMs = 10000; // 10s
const std::string ProducerImpl::kFinishStreamKeyword = "asapo_finish_stream";
const std::string ProducerImpl::kNoNextStreamKeyword = "asapo_no_next";


ProducerImpl::ProducerImpl(std::string endpoint, uint8_t n_processing_threads, uint64_t timeout_ms,
                           asapo::RequestHandlerType type):
    log__{GetDefaultProducerLogger()}, timeout_ms_{timeout_ms} {
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

GenericRequestHeader ProducerImpl::GenerateNextSendRequest(const MessageHeader& message_header, std::string stream,
                                                           uint64_t ingest_mode) {
    GenericRequestHeader request{kOpcodeTransferData, message_header.message_id, message_header.data_size,
                                 message_header.user_metadata.size(), message_header.file_name, stream};
    if (message_header.dataset_substream != 0) {
        request.op_code = kOpcodeTransferDatasetData;
        request.custom_data[kPosDataSetId] = message_header.dataset_substream;
        request.custom_data[kPosDataSetSize] = message_header.dataset_size;
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

    if (ingest_mode & IngestModeFlags::kTransferData &&
        !(ingest_mode & (IngestModeFlags::kStoreInDatabase | IngestModeFlags::kStoreInFilesystem))) {
        return ProducerErrorTemplates::kWrongInput.Generate("wrong ingest mode");
    }

    if (ingest_mode & IngestModeFlags::kTransferMetaDataOnly &&
        (ingest_mode & IngestModeFlags::kStoreInFilesystem)) {
        return ProducerErrorTemplates::kWrongInput.Generate("wrong ingest mode");
    }

    return nullptr;
}

Error CheckProducerRequest(const MessageHeader& message_header, uint64_t ingest_mode) {
    if (message_header.file_name.size() > kMaxMessageSize) {
        return ProducerErrorTemplates::kWrongInput.Generate("too long filename");
    }

    if (message_header.file_name.empty() ) {
        return ProducerErrorTemplates::kWrongInput.Generate("empty filename");
    }

    if (message_header.dataset_substream > 0 && message_header.dataset_size == 0) {
        return ProducerErrorTemplates::kWrongInput.Generate("dataset dimensions");
    }

    if (message_header.message_id == 0) {
        return ProducerErrorTemplates::kWrongInput.Generate("data tuple id should be positive");
    }

    return CheckIngestMode(ingest_mode);
}

Error ProducerImpl::Send(const MessageHeader& message_header,
                         std::string stream,
                         MessageData data,
                         std::string full_path,
                         uint64_t ingest_mode,
                         RequestCallback callback,
                         bool manage_data_memory) {
    auto err = CheckProducerRequest(message_header, ingest_mode);
    if (err) {
        if (!manage_data_memory) {
            data.release();
        }
        log__->Error("error checking request - " + err->Explain());
        return err;
    }

    auto request_header = GenerateNextSendRequest(message_header, std::move(stream), ingest_mode);

    return request_pool__->AddRequest(std::unique_ptr<ProducerRequest> {new ProducerRequest{source_cred_string_, std::move(request_header),
                std::move(data), std::move(message_header.user_metadata), std::move(full_path), callback, manage_data_memory, timeout_ms_}
    });

}

bool WandTransferData(uint64_t ingest_mode) {
    return ingest_mode & IngestModeFlags::kTransferData;
}

Error CheckData(uint64_t ingest_mode, const MessageHeader& message_header, const MessageData* data) {
    if (WandTransferData(ingest_mode)) {
        if (*data == nullptr) {
            return ProducerErrorTemplates::kWrongInput.Generate("need data for this ingest mode");
        }
        if (message_header.data_size == 0) {
            return ProducerErrorTemplates::kWrongInput.Generate("zero data size");
        }
    }
    return nullptr;
}

Error ProducerImpl::Send(const MessageHeader& message_header, MessageData data,
                         uint64_t ingest_mode, RequestCallback callback) {
    return Send(message_header, kDefaultStream, std::move(data), ingest_mode, callback);
}

Error ProducerImpl::Send(const MessageHeader& message_header,
                         std::string stream,
                         MessageData data,
                         uint64_t ingest_mode,
                         RequestCallback callback) {
    if (auto err = CheckData(ingest_mode, message_header, &data)) {
        return err;
    }
    return Send(message_header, std::move(stream), std::move(data), "", ingest_mode, callback, true);

}

Error ProducerImpl::SendStreamFinishedFlag(std::string stream, uint64_t last_id, std::string next_stream,
                                              RequestCallback callback) {
    MessageHeader message_header;
    message_header.file_name = kFinishStreamKeyword;
    message_header.data_size = 0;
    message_header.message_id = last_id + 1;
    if (next_stream.empty()) {
        next_stream = kNoNextStreamKeyword;
    }
    message_header.user_metadata =  std::string("{\"next_stream\":") + "\"" + next_stream + "\"}";
    return Send(message_header, std::move(stream), nullptr, "", IngestModeFlags::kTransferMetaDataOnly, callback, true);
}

Error ProducerImpl::SendFile(const MessageHeader& message_header, std::string full_path, uint64_t ingest_mode,
                                 RequestCallback callback) {
    return SendFile(message_header, kDefaultStream, std::move(full_path), ingest_mode, callback);
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

    if (source_cred.data_source.empty()) {
        source_cred.data_source = SourceCredentials::kDefaultStream;
    }

    if (source_cred.beamline.empty()) {
        source_cred.beamline = SourceCredentials::kDefaultBeamline;
    }

    if (source_cred.beamtime_id.empty()) {
        source_cred.beamtime_id = SourceCredentials::kDefaultBeamtimeId;
    }

    if (source_cred.beamtime_id == SourceCredentials::kDefaultBeamtimeId
            && source_cred.beamline == SourceCredentials::kDefaultBeamline) {
        log__->Error("beamtime or beamline should be set");
        source_cred_string_ = "";
        return ProducerErrorTemplates::kWrongInput.Generate("beamtime or beamline should be set");
    }

    source_cred_string_ = source_cred.GetString();
    if (source_cred_string_.size()  + source_cred.user_token.size() > kMaxMessageSize) {
        log__->Error("credentials string is too long - " + source_cred_string_);
        source_cred_string_ = "";
        return ProducerErrorTemplates::kWrongInput.Generate("credentials string is too long");
    }

    return nullptr;
}

Error ProducerImpl::SendMetadata(const std::string& metadata, RequestCallback callback) {
    GenericRequestHeader request_header{kOpcodeTransferMetaData, 0, metadata.size(), 0, "beamtime_global.meta"};
    request_header.custom_data[kPosIngestMode] = asapo::IngestModeFlags::kTransferData | asapo::IngestModeFlags::kStoreInDatabase;
    MessageData data{new uint8_t[metadata.size()]};
    strncpy((char*)data.get(), metadata.c_str(), metadata.size());
    return request_pool__->AddRequest(std::unique_ptr<ProducerRequest> {new ProducerRequest{source_cred_string_, std::move(request_header),
                std::move(data), "", "", callback, true, timeout_ms_}
    });
}

Error ProducerImpl::Send__(const MessageHeader& message_header,
                                  std::string stream,
                                  void* data,
                                  uint64_t ingest_mode,
                                  RequestCallback callback) {
    MessageData data_wrapped = MessageData{(uint8_t*)data};

    if (auto err = CheckData(ingest_mode, message_header, &data_wrapped)) {
        data_wrapped.release();
        return err;
    }

    return Send(std::move(message_header), std::move(stream), std::move(data_wrapped), "", ingest_mode, callback, false);
}

Error ProducerImpl::Send__(const MessageHeader& message_header,
                                  void* data,
                                  uint64_t ingest_mode,
                                  RequestCallback callback) {
    return Send__(message_header, kDefaultStream, data, ingest_mode, callback);
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
Error ProducerImpl::SendFile(const MessageHeader& message_header,
                                        std::string stream,
                                        std::string full_path,
                                        uint64_t ingest_mode,
                                        RequestCallback callback) {
    if (full_path.empty()) {
        return ProducerErrorTemplates::kWrongInput.Generate("empty filename");
    }

    return Send(message_header, std::move(stream), nullptr, std::move(full_path), ingest_mode, callback, true);

}

using RequestCallbackWithPromise = void (*)(std::shared_ptr<std::promise<StreamInfoResult>>,
                                            RequestCallbackPayload header, Error err);

RequestCallback unwrap_callback(RequestCallbackWithPromise callback,
                                std::unique_ptr<std::promise<StreamInfoResult>> promise) {
    auto shared_promise = std::shared_ptr<std::promise<StreamInfoResult>>(std::move(promise));
    RequestCallback wrapper = [ = ](RequestCallbackPayload payload, Error err) -> void {
        callback(shared_promise, std::move(payload), std::move(err));
    };
    return wrapper;
}

void ActivatePromise(std::shared_ptr<std::promise<StreamInfoResult>> promise, RequestCallbackPayload payload,
                     Error err) {
    StreamInfoResult res;
    if (err == nullptr) {
        auto ok = res.sinfo.SetFromJson(payload.response,true);
        res.err = ok ? nullptr : ProducerErrorTemplates::kInternalServerError.Generate(
                      std::string("cannot read JSON string from server response: ") + payload.response).release();
    } else {
        res.err = err.release();
    }
    try {
        promise->set_value(res);
    } catch(...) {}
}

StreamInfo GetInfoFromCallback(std::future<StreamInfoResult>* promiseResult, uint64_t timeout_ms, Error* err) {
    try {
        auto status = promiseResult->wait_for(std::chrono::milliseconds(timeout_ms));
        if (status == std::future_status::ready) {
            auto res = promiseResult->get();
            if (res.err == nullptr) {
                return res.sinfo;
            } else {
                (*err).reset(res.err);
                return StreamInfo{};
            }
        }
    } catch(...) {}

    *err = ProducerErrorTemplates::kTimeout.Generate();
    return StreamInfo{};
}


GenericRequestHeader CreateRequestHeaderFromOp(StreamRequestOp op,std::string stream) {
    switch (op) {
        case StreamRequestOp::kStreamInfo:
            return GenericRequestHeader{kOpcodeStreamInfo, 0, 0, 0, "", stream};
        case StreamRequestOp::kLastStream:
            return GenericRequestHeader{kOpcodeLastStream, 0, 0, 0, "", ""};
    }
}

StreamInfo ProducerImpl::StreamRequest(StreamRequestOp op,std::string stream, uint64_t timeout_ms, Error* err) const {
    auto header = CreateRequestHeaderFromOp(op,stream);
    std::unique_ptr<std::promise<StreamInfoResult>> promise {new std::promise<StreamInfoResult>};
    std::future<StreamInfoResult> promiseResult = promise->get_future();

    *err = request_pool__->AddRequest(std::unique_ptr<ProducerRequest> {new ProducerRequest{source_cred_string_, std::move(header),
                                                                                            nullptr, "", "",
                                                                                            unwrap_callback(ActivatePromise, std::move(promise)), true,
                                                                                            timeout_ms}
    }, true);
    if (*err) {
        return StreamInfo{};
    }
    return GetInfoFromCallback(&promiseResult, timeout_ms + 2000,
                               err); // we give two more sec for request to exit by timeout
}

StreamInfo ProducerImpl::GetStreamInfo(std::string stream, uint64_t timeout_ms, Error* err) const {
    return StreamRequest(StreamRequestOp::kStreamInfo,stream,timeout_ms,err);
}

StreamInfo ProducerImpl::GetStreamInfo(uint64_t timeout_ms, Error* err) const {
    return GetStreamInfo(kDefaultStream, timeout_ms, err);
}

StreamInfo ProducerImpl::GetLastStream(uint64_t timeout_ms, Error* err) const {
    return StreamRequest(StreamRequestOp::kLastStream,"",timeout_ms,err);
}

uint64_t ProducerImpl::GetRequestsQueueVolumeMb() {
    return request_pool__->UsedMemoryInPool()/1000000;
}

void ProducerImpl::SetRequestsQueueLimits(uint64_t size, uint64_t volume) {
    request_pool__->SetLimits(RequestPoolLimits{size,volume});
}


}