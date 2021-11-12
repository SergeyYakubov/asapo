#include "request.h"
#include "asapo/io/io_factory.h"
#include "request_handler/request_handler_db_check_request.h"

namespace asapo {

Request::Request(const GenericRequestHeader& header,
                 SocketDescriptor socket_fd, std::string origin_uri, DataCache* cache,
                 const RequestHandlerDbCheckRequest* db_check_handler) : io__{GenerateDefaultIO()},
    cache__{cache}, request_header_(header),
    socket_fd_{socket_fd}, origin_uri_{std::move(origin_uri)},
    check_duplicate_request_handler_{db_check_handler} {
    origin_host_ = HostFromUri(origin_uri_);
}

Error Request::PrepareDataBufferAndLockIfNeeded() {
    if (cache__ == nullptr) {
        try {
            data_buffer_.reset(new uint8_t[(size_t)request_header_.data_size]);
        } catch(std::exception& e) {
            auto err = GeneralErrorTemplates::kMemoryAllocationError.Generate(
                std::string("cannot allocate memory for request"));
            err->AddDetails("reason", e.what())->AddDetails("size", std::to_string(request_header_.data_size));
            return err;
        }
    } else {
        CacheMeta* slot;
        data_ptr = cache__->GetFreeSlotAndLock(request_header_.data_size, &slot);
        if (data_ptr) {
            slot_meta_ = slot;
        } else {
            auto err = GeneralErrorTemplates::kMemoryAllocationError.Generate("cannot allocate slot in cache");
            err->AddDetails("size", std::to_string(request_header_.data_size));
            return err;
        }
    }
    return nullptr;
}


Error Request::Handle(ReceiverStatistics* statistics) {
    for (auto handler : handlers_) {
        statistics->StartTimer(handler->GetStatisticEntity());
        auto err = handler->ProcessRequest(this);
        if (err) {
            return err;
        }
        statistics->StopTimer();
    }
    return nullptr;
}

const RequestHandlerList& Request::GetListHandlers() const {
    return handlers_;
}


void Request::AddHandler(const ReceiverRequestHandler* handler) {
    handlers_.emplace_back(handler);
}


uint64_t Request::GetDataID() const {
    return request_header_.data_id;
}


uint64_t Request::GetDataSize() const {
    return request_header_.data_size;
}

void* Request::GetData() const {
    if (cache__) {
        return data_ptr;
    } else {
        return data_buffer_.get();
    }

}

std::string Request::GetFileName() const {
    std::string orig_name = request_header_.message;
    if (kPathSeparator == '/') {
        std::replace(orig_name.begin(), orig_name.end(), '\\', kPathSeparator);
    } else {
        std::replace(orig_name.begin(), orig_name.end(), '/', kPathSeparator);
    }

    return orig_name;
}

std::string Request::GetStream() const {
    return request_header_.stream;
}

std::string Request::GetApiVersion() const {
    return request_header_.api_version;
}

const std::string& Request::GetOriginUri() const {
    return origin_uri_;
}
const std::string& Request::GetBeamtimeId() const {
    return beamtime_id_;
}
void Request::SetBeamtimeId(std::string beamtime_id) {
    beamtime_id_ = std::move(beamtime_id);
}

Opcode Request::GetOpCode() const {
    return request_header_.op_code;
}
const char* Request::GetMessage() const {
    return request_header_.message;
}

void Request::SetBeamline(std::string beamline) {
    beamline_ = std::move(beamline);
}

const std::string& Request::GetBeamline() const {
    return beamline_;
}

uint64_t Request::GetSlotId() const {
    if (slot_meta_) {
        return slot_meta_->id;
    } else {
        return 0;
    }
}

const std::string& Request::GetMetaData() const {
    return metadata_;
}

const CustomRequestData& Request::GetCustomData() const {
    return request_header_.custom_data;
}
const std::string& Request::GetDataSource() const {
    return data_source_;
}
void Request::SetDataSource(std::string data_source) {
    data_source_ = std::move(data_source);
}

void Request::UnlockDataBufferIfNeeded() {
    if (slot_meta_) {
        cache__->UnlockSlot(slot_meta_);
    }
}
SocketDescriptor Request::GetSocket() const {
    return socket_fd_;
}

void Request::SetMetadata(std::string metadata) {
    metadata_ = std::move(metadata);
}

uint64_t Request::GetMetaDataSize() const {
    return request_header_.meta_size;
}

void Request::SetOnlinePath(std::string path) {
    online_path_ = std::move(path);
}

void Request::SetOfflinePath(std::string path) {
    offline_path_ = std::move(path);
}

const std::string& Request::GetOnlinePath() const {
    return online_path_;
}

const std::string& Request::GetOfflinePath() const {
    return offline_path_;
}

bool Request::WasAlreadyProcessed() const {
    return already_processed_;
}

void Request::SetAlreadyProcessedFlag() {
    already_processed_ = true;
}

void Request::SetResponseMessage(std::string message, ResponseMessageType type) {
    response_message_ = std::move(message);
    response_message_type_ = type;
}

const std::string& Request::GetResponseMessage() const {
    return response_message_;
}

ResponseMessageType Request::GetResponseMessageType() const {
    return response_message_type_;
}

Error Request::CheckForDuplicates()  {
    return check_duplicate_request_handler_->ProcessRequest(this);
}
void Request::SetSourceType(SourceType type) {
    source_type_ = type;
}
SourceType Request::GetSourceType() const {
    return source_type_;
}

const std::string& Request::GetOriginHost() const {
    return origin_host_;
}

}
