#include "request.h"
#include "io/io_factory.h"

#include "receiver_config.h"
namespace asapo {

Request::Request(const GenericRequestHeader& header,
                 SocketDescriptor socket_fd, std::string origin_uri, DataCache* cache) : io__{GenerateDefaultIO()},
    cache__{cache}, request_header_(header),
    socket_fd_{socket_fd}, origin_uri_{std::move(origin_uri)} {
}

Error Request::PrepareDataBuffer() {
    if (cache__ == nullptr) {
        try {
            data_buffer_.reset(new uint8_t[(size_t)request_header_.data_size]);
        } catch(std::exception& e) {
            auto err = ErrorTemplates::kMemoryAllocationError.Generate();
            err->Append(e.what());
            return err;
        }
    } else {
        CacheMeta* slot;
        data_ptr = cache__->GetFreeSlotAndLock(request_header_.data_size, &slot);
        if (data_ptr) {
            slot_meta_ = slot;
        } else {
            return ErrorTemplates::kMemoryAllocationError.Generate("cannot allocate slot in cache");
        }
    }
    return nullptr;
}

Error Request::ReceiveMetaData() {
    if (request_header_.meta_size == 0) {
        return nullptr;
    }

    Error err;
    auto buf = std::unique_ptr<uint8_t[]> {new uint8_t[(size_t)request_header_.meta_size]};
    io__->Receive(socket_fd_, (void*) buf.get(), (size_t) request_header_.meta_size, &err);
    if (err) {
        return err;
    }
    metadata_.assign((char*)buf.get(), request_header_.meta_size);
    return nullptr;
}


Error Request::ReceiveData() {
    if (request_header_.data_size == 0) {
        return nullptr;
    }

    auto err = PrepareDataBuffer();
    if (err) {
        return err;
    }
    io__->Receive(socket_fd_, GetData(), (size_t) request_header_.data_size, &err);
    if (slot_meta_) {
        cache__->UnlockSlot(slot_meta_);
    }
    return err;
}


Error Request::ReceiveRequestContent(ReceiverStatistics* statistics) {
    statistics->StartTimer(StatisticEntity::kNetwork);
    auto err = ReceiveData();
    if (err) {
        return err;
    }

    err = ReceiveMetaData();
    if (err) {
        return err;
    }

    statistics->StopTimer();

    return nullptr;
}



Error Request::Handle(ReceiverStatistics* statistics) {
    auto err = ReceiveRequestContent(statistics);
    if (err) {
        return err;
    }

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

}