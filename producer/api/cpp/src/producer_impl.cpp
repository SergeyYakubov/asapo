#include <iostream>
#include <cstring>
#include <future>

#include "producer_impl.h"
#include "producer_logger.h"
#include "asapo/producer/producer_error.h"
#include "producer_request_handler_factory.h"
#include "producer_request.h"
#include "asapo/common/data_structs.h"
#include "asapo/request/request_pool_error.h"
#include "asapo/http_client/http_client.h"
#include "asapo/common/internal/version.h"
#include <asapo/io/io_factory.h>

namespace asapo {

const size_t ProducerImpl::kDiscoveryServiceUpdateFrequencyMs = 10000; // 10s

ProducerImpl::ProducerImpl(std::string endpoint, uint8_t n_processing_threads, uint64_t timeout_ms,
                           asapo::RequestHandlerType type) :
                           log__{GetDefaultProducerLogger()}, io__{GenerateDefaultIO()}, httpclient__{DefaultHttpClient()}, timeout_ms_{timeout_ms}, endpoint_{endpoint} {
    switch (type) {
    case RequestHandlerType::kTcp:
        discovery_service_.reset(new ReceiverDiscoveryService{endpoint,
                                 ProducerImpl::kDiscoveryServiceUpdateFrequencyMs});
        request_handler_factory_.reset(new ProducerRequestHandlerFactory{discovery_service_.get()});
        break;
    case RequestHandlerType::kFilesystem:
        request_handler_factory_.reset(new ProducerRequestHandlerFactory{endpoint});
        break;
    }
    request_pool__.reset(new RequestPool{n_processing_threads, request_handler_factory_.get(), log__});

    source_cred_string_using_new_format_ = false;
}

GenericRequestHeader ProducerImpl::GenerateNextSendRequest(const MessageHeader& message_header, std::string stream,
        uint64_t ingest_mode) {
    GenericRequestHeader request{kOpcodeTransferData, message_header.message_id, message_header.data_size,
                                 message_header.user_metadata.size(), message_header.file_name, std::move(stream)};
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

Error CheckFileNameInRequest(const MessageHeader& message_header) {
    if (message_header.file_name.size() > kMaxMessageSize) {
        return ProducerErrorTemplates::kWrongInput.Generate("too long filename");
    }

    if (message_header.file_name.empty()) {
        return ProducerErrorTemplates::kWrongInput.Generate("empty filename");
    }
    return nullptr;
}

Error CheckDatasetInRequest(const MessageHeader& message_header) {
    if (!message_header.dataset_substream) {
        return nullptr;
    }

    if (message_header.dataset_size == 0) {
        return ProducerErrorTemplates::kWrongInput.Generate("dataset dimensions");
    }

    if (message_header.auto_id) {
        return ProducerErrorTemplates::kWrongInput.Generate("auto id mode not implemented for datasets");
    }

    return nullptr;
}

Error CheckMessageIdInRequest(const MessageHeader& message_header) {
    if (message_header.auto_id) {
        if (message_header.message_id) {
            return ProducerErrorTemplates::kWrongInput.Generate("message id should be 0 for auto id mode");
        }
    } else {
        if (message_header.message_id == 0) {
            return ProducerErrorTemplates::kWrongInput.Generate("message id should be positive");
        }
    }
    return nullptr;
}

Error CheckProducerRequest(const MessageHeader& message_header, uint64_t ingest_mode, const std::string& stream) {
    if (stream.empty()) {
        return ProducerErrorTemplates::kWrongInput.Generate("stream empty");
    }

    if (auto err = CheckFileNameInRequest(message_header)) {
        return err;
    }

    if (auto err = CheckDatasetInRequest(message_header)) {
        return err;
    }

    if (auto err = CheckMessageIdInRequest(message_header)) {
        return err;
    }

    return CheckIngestMode(ingest_mode);
}

Error HandleErrorFromPool(Error original_error, bool manage_data_memory) {
    if (original_error == nullptr) {
        return nullptr;
    }
    Error producer_error = ProducerErrorTemplates::kRequestPoolIsFull.Generate(original_error->Explain());
    auto err_data = static_cast<OriginalRequest*>(original_error->GetCustomData());
    if (!err_data) {
        return producer_error;
    }
    auto producer_request = static_cast<ProducerRequest*>(err_data->request.get());
    if (!producer_request) {
        return producer_error;
    }
    MessageData original_data = std::move(producer_request->data);
    if (original_data == nullptr) {
        return producer_error;
    }
    if (!manage_data_memory) {
        original_data.release();
    } else {
        OriginalData* original = new asapo::OriginalData{};
        original->data = std::move(original_data);
        producer_error->SetCustomData(std::unique_ptr<asapo::CustomErrorData> {original});
    }
    return producer_error;
}

Error HandleInputError(Error original_error, MessageData data, bool manage_data_memory) {
    if (data == nullptr) {
        return original_error;
    }
    if (!manage_data_memory) {
        data.release();
        return original_error;
    }

    OriginalData* original = new asapo::OriginalData{};
    original->data = std::move(data);
    original_error->SetCustomData(std::unique_ptr<asapo::CustomErrorData> {original});
    return original_error;
}

Error ProducerImpl::Send(const MessageHeader& message_header,
                         std::string stream,
                         MessageData data,
                         std::string full_path,
                         uint64_t ingest_mode,
                         RequestCallback callback,
                         bool manage_data_memory) {
    auto err = CheckProducerRequest(message_header, ingest_mode, stream);
    if (err) {
        log__->Error("error checking request - " + err->Explain());
        return HandleInputError(std::move(err), std::move(data), manage_data_memory);
    }

    auto request_header = GenerateNextSendRequest(message_header, std::move(stream), ingest_mode);

    err = request_pool__->AddRequest(std::unique_ptr<ProducerRequest> {
        new ProducerRequest{
                source_cred_string_using_new_format_, source_cred_string_,
                std::move(request_header), std::move(data), std::move(message_header.user_metadata),
                std::move(full_path), callback,
                manage_data_memory, timeout_ms_}
    });

    return HandleErrorFromPool(std::move(err), manage_data_memory);
}

bool WantTransferData(uint64_t ingest_mode) {
    return ingest_mode & IngestModeFlags::kTransferData;
}

Error CheckData(uint64_t ingest_mode, const MessageHeader& message_header, const MessageData* data) {
    if (WantTransferData(ingest_mode)) {
        if (*data == nullptr) {
            return ProducerErrorTemplates::kWrongInput.Generate("need data for this ingest mode");
        }
        if (message_header.data_size == 0) {
            return ProducerErrorTemplates::kWrongInput.Generate("zero data size");
        }
    }
    return nullptr;
}

Error ProducerImpl::Send(const MessageHeader& message_header,
                         MessageData data,
                         uint64_t ingest_mode,
                         std::string stream,
                         RequestCallback callback) {
    if (auto err = CheckData(ingest_mode, message_header, &data)) {
        return HandleInputError(std::move(err), std::move(data), true);
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
    message_header.user_metadata = std::string("{\"next_stream\":") + "\"" + next_stream + "\"}";
    return Send(message_header, std::move(stream), nullptr, "", IngestModeFlags::kTransferMetaDataOnly, callback, true);
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

    Error err = RefreshSourceCredentialString(source_cred);
    if (!err) {
        last_creds_.reset(new SourceCredentials{source_cred});
    }
    return err;
}

Error ProducerImpl::EnableNewMonitoringApiFormat(bool enabled) {
    source_cred_string_using_new_format_ = enabled;
    Error err;
    if (last_creds_) {
        err = RefreshSourceCredentialString(*last_creds_);
    }
    return err;
}

Error ProducerImpl::RefreshSourceCredentialString(SourceCredentials source_cred) {
    if (source_cred.instance_id.empty()) {
        source_cred.instance_id = SourceCredentials::kDefaultInstanceId;
    }

    if (source_cred.pipeline_step.empty()) {
        source_cred.pipeline_step = SourceCredentials::kDefaultPipelineStep;
    }

    if (source_cred.data_source.empty()) {
        source_cred.data_source = SourceCredentials::kDefaultDataSource;
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

    if (source_cred.instance_id == SourceCredentials::kDefaultInstanceId) {
        Error err;
        std::string hostname = io__->GetHostName(&err);

        if (err) {
            hostname = "hostnameerror";
        }

        source_cred.instance_id = hostname + "_" + std::to_string(io__->GetCurrentPid());
    }

    if (source_cred.pipeline_step == SourceCredentials::kDefaultPipelineStep) {
        source_cred.pipeline_step = "DefaultStep";
    }

    source_cred_string_ = source_cred.GetString(source_cred_string_using_new_format_ ? SourceCredentialsVersion::NewVersion : SourceCredentialsVersion::OldVersion);

    if (source_cred_string_.size() + source_cred.user_token.size() > kMaxMessageSize) {
        log__->Error("credentials string is too long - " + source_cred_string_);
        source_cred_string_ = "";
        return ProducerErrorTemplates::kWrongInput.Generate("credentials string is too long");
    }

    return nullptr;
}

Error ProducerImpl::SendMetadata(const std::string& metadata, RequestCallback callback) {
    auto mode = MetaIngestMode{MetaIngestOp::kReplace, true};
    return SendBeamtimeMetadata(metadata, mode, callback);
}

Error ProducerImpl::Send__(const MessageHeader& message_header,
                           void* data,
                           uint64_t ingest_mode,
                           std::string stream,
                           RequestCallback callback) {
    MessageData data_wrapped = MessageData{(uint8_t*) data};

    if (auto err = CheckData(ingest_mode, message_header, &data_wrapped)) {
        data_wrapped.release();
        return err;
    }

    return Send(std::move(message_header),
                std::move(stream),
                std::move(data_wrapped),
                "",
                ingest_mode,
                callback,
                false);
}

uint64_t ProducerImpl::GetRequestsQueueSize() {
    return request_pool__->NRequestsInPool();
}

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
                             std::string full_path,
                             uint64_t ingest_mode,
                             std::string stream,
                             RequestCallback callback) {
    if (full_path.empty()) {
        return ProducerErrorTemplates::kWrongInput.Generate("empty filename");
    }

    return Send(message_header, std::move(stream), nullptr, std::move(full_path), ingest_mode, callback, true);

}

template<class T>
using RequestCallbackWithPromise = void (*)(std::shared_ptr<std::promise<T>>,
                                            RequestCallbackPayload header, Error err);

template<class T>
RequestCallback unwrap_callback(RequestCallbackWithPromise<T> callback,
                                std::unique_ptr<std::promise<T>> promise) {
    auto shared_promise = std::shared_ptr<std::promise<T>>(std::move(promise));
    RequestCallback wrapper = [ = ](RequestCallbackPayload payload, Error err) -> void {
        callback(shared_promise, std::move(payload), std::move(err));
    };
    return wrapper;
}

void ActivatePromiseForReceiverResponse(std::shared_ptr<std::promise<ReceiverResponse>> promise,
                                        RequestCallbackPayload payload,
                                        Error err) {
    ReceiverResponse res;
    if (err == nullptr) {
        res.payload = payload.response;
        res.err = nullptr;
    } else {
        res.err = err.release();
    }
    try {
        promise->set_value(res);
    } catch (...) {}
}

template<class T>
T GetResultFromCallback(std::future<T>* promiseResult, uint64_t timeout_ms, Error* err) {
    try {
        auto status = promiseResult->wait_for(std::chrono::milliseconds(timeout_ms));
        if (status == std::future_status::ready) {
            return promiseResult->get();
        }
    } catch (...) {}

    *err = ProducerErrorTemplates::kTimeout.Generate();
    return T{};
}

GenericRequestHeader CreateRequestHeaderFromOp(StreamRequestOp op, std::string stream) {
    switch (op) {
    case StreamRequestOp::kStreamInfo:
        return GenericRequestHeader{kOpcodeStreamInfo, 0, 0, 0, "", stream};
    case StreamRequestOp::kLastStream:
        return GenericRequestHeader{kOpcodeLastStream, 0, 0, 0, "", ""};
    }
    return GenericRequestHeader{};
}

std::string ProducerImpl::BlockingRequest(GenericRequestHeader header, uint64_t timeout_ms, Error* err) const {
    std::unique_ptr<std::promise<ReceiverResponse>> promise{new std::promise<ReceiverResponse>};
    std::future<ReceiverResponse> promiseResult = promise->get_future();

    *err = request_pool__->AddRequest(std::unique_ptr<ProducerRequest> {
        new ProducerRequest{
                source_cred_string_using_new_format_,
                source_cred_string_, std::move(header),
                nullptr, "", "",
                unwrap_callback(
                ActivatePromiseForReceiverResponse,
                std::move(promise)), true,
                timeout_ms}
    }, true);
    if (*err) {
        return "";
    }

    auto res = GetResultFromCallback<ReceiverResponse>(&promiseResult, timeout_ms + 2000,
               err); // we give two more sec for request to exit by timeout
    if (*err) {
        return "";
    }


    if (res.err == nullptr) {
        return res.payload;
    } else {
        (*err).reset(res.err);
        return "";
    }
}

StreamInfo ProducerImpl::StreamRequest(StreamRequestOp op, std::string stream, uint64_t timeout_ms, Error* err) const {
    auto header = CreateRequestHeaderFromOp(op, stream);

    auto response = BlockingRequest(std::move(header), timeout_ms, err);
    if (*err) {
        return StreamInfo{};
    }

    StreamInfo res;
    if (!res.SetFromJson(response)) {
        *err = ProducerErrorTemplates::kInternalServerError.Generate(
                   std::string("cannot read JSON string from server response: ") + response);
        return StreamInfo{};
    }

    *err = nullptr;
    return res;
}

StreamInfo ProducerImpl::GetStreamInfo(std::string stream, uint64_t timeout_ms, Error* err) const {
    if (stream.empty()) {
        *err = ProducerErrorTemplates::kWrongInput.Generate("stream empty");
        return {};
    }
    return StreamRequest(StreamRequestOp::kStreamInfo, stream, timeout_ms, err);
}

StreamInfo ProducerImpl::GetLastStream(uint64_t timeout_ms, Error* err) const {
    return StreamRequest(StreamRequestOp::kLastStream, "", timeout_ms, err);
}

uint64_t ProducerImpl::GetRequestsQueueVolumeMb() {
    return request_pool__->UsedMemoryInPool() / 1000000;
}

void ProducerImpl::SetRequestsQueueLimits(uint64_t size, uint64_t volume) {
    request_pool__->SetLimits(RequestPoolLimits{size, volume});
}

Error ProducerImpl::GetVersionInfo(std::string* client_info, std::string* server_info, bool* supported) const {
    if (client_info == nullptr && server_info == nullptr && supported == nullptr) {
        return ProducerErrorTemplates::kWrongInput.Generate("missing parameters");
    }
    if (client_info != nullptr) {
        *client_info =
            "software version: " + std::string(kVersion) + ", producer protocol: " + kProducerProtocol.GetVersion();
    }

    if (server_info != nullptr || supported != nullptr) {
        return GetServerVersionInfo(server_info, supported);
    }
    return nullptr;
}

Error ProducerImpl::GetServerVersionInfo(std::string* server_info,
                                         bool* supported) const {
    auto endpoint = endpoint_ + "/asapo-discovery/" + kProducerProtocol.GetDiscoveryVersion() +
                    "/version?client=producer&protocol=" + kProducerProtocol.GetVersion();
    HttpCode code;
    Error err;
    auto response = httpclient__->Get(endpoint, &code, &err);
    if (err) {
        return err;
    }
    return ExtractVersionFromResponse(response, "producer", server_info, supported);
}

Error ProducerImpl::DeleteStream(std::string stream, uint64_t timeout_ms, DeleteStreamOptions options) const {
    auto header = GenericRequestHeader{kOpcodeDeleteStream, 0, 0, 0, "", stream};
    header.custom_data[0] = options.Encode();

    Error err;
    BlockingRequest(std::move(header), timeout_ms, &err);
    return err;
}

Error ProducerImpl::SendBeamtimeMetadata(const std::string& metadata, MetaIngestMode mode, RequestCallback callback) {
    return SendMeta(metadata, mode, "", callback);
}

Error ProducerImpl::SendStreamMetadata(const std::string& metadata,
                                       MetaIngestMode mode,
                                       const std::string& stream,
                                       RequestCallback callback) {
    if (stream.empty()) {
        return ProducerErrorTemplates::kWrongInput.Generate("stream is empty");
    }
    return SendMeta(metadata, mode, stream, callback);
}

Error ProducerImpl::SendMeta(const std::string& metadata,
                             MetaIngestMode mode,
                             std::string stream,
                             RequestCallback callback) {
    GenericRequestHeader request_header{kOpcodeTransferMetaData, 0, metadata.size(), 0,
                                        stream.empty() ? "beamtime_global.meta" : stream + ".meta",
                                        stream};
    request_header.custom_data[kPosIngestMode] = asapo::IngestModeFlags::kTransferData |
                                                 asapo::IngestModeFlags::kStoreInDatabase;
    request_header.custom_data[kPosMetaIngestMode] = mode.Encode();
    MessageData data{new uint8_t[metadata.size()]};
    strncpy((char*) data.get(), metadata.c_str(), metadata.size());
    auto err = request_pool__->AddRequest(std::unique_ptr<ProducerRequest> {
        new ProducerRequest{
            source_cred_string_using_new_format_,
            source_cred_string_, std::move(request_header),
            std::move(data), "", "", callback, true, timeout_ms_}
    });
    return HandleErrorFromPool(std::move(err), true);
}

std::string ProducerImpl::GetStreamMeta(const std::string& stream, uint64_t timeout_ms, Error* err) const {
    return GetMeta(stream, timeout_ms, err);
}

std::string ProducerImpl::GetBeamtimeMeta(uint64_t timeout_ms, Error* err) const {
    return GetMeta("", timeout_ms, err);
}

std::string ProducerImpl::GetMeta(const std::string& stream, uint64_t timeout_ms, Error* err) const {
    auto header =  GenericRequestHeader{kOpcodeGetMeta, 0, 0, 0, "", stream};
    auto response = BlockingRequest(std::move(header), timeout_ms, err);
    if (*err) {
        return "";
    }
    *err = nullptr;
    return response;
}

}
