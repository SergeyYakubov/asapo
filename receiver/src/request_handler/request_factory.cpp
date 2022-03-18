#include "request_factory.h"

#include "../receiver_config.h"

namespace asapo {

bool NeedFileWriteHandler(const GenericRequestHeader& request_header) {
    return static_cast<bool>(request_header.custom_data[kPosIngestMode] & IngestModeFlags::kStoreInFilesystem);
}

bool NeedDbHandler(const GenericRequestHeader& request_header) {
    return static_cast<bool>(request_header.custom_data[kPosIngestMode] & IngestModeFlags::kStoreInDatabase) ||
           (request_header.custom_data[kPosIngestMode] == asapo::IngestModeFlags::kTransferMetaDataOnly);
}

bool RequestFactory::ReceiveDirectToFile(const GenericRequestHeader& request_header) const {
    return request_header.data_size > GetReceiverConfig()->receive_to_disk_threshold_mb * 1024 * 1024;
}

Error RequestFactory::AddReceiveWriteHandlers(std::unique_ptr<Request>& request,
                                              const GenericRequestHeader& request_header) const {
    if (ReceiveDirectToFile(request_header)) {
        return AddReceiveDirectToFileHandler(request, request_header);
    } else {
        AddReceiveViaBufferHandlers(request, request_header);
        return nullptr;
    }
}

void RequestFactory::AddReceiveViaBufferHandlers(std::unique_ptr<Request>& request,
                                                 const GenericRequestHeader& request_header) const {
    request->AddHandler(&request_handler_receivedata_);
    if (NeedFileWriteHandler(request_header)) {
        request->AddHandler(&request_handler_filewrite_);
        if (GetReceiverConfig()->kafka_config.enabled) {
            request->AddHandler(&request_handler_kafka_notify_);
        }
    }
}

Error RequestFactory::AddReceiveDirectToFileHandler(std::unique_ptr<Request>& request,
        const GenericRequestHeader& request_header) const {
    if (!(request_header.custom_data[kPosIngestMode] & kStoreInFilesystem)) {
        return ReceiverErrorTemplates::kBadRequest.Generate(
                   "ingest mode should include kStoreInFilesystem for large files ");
    }
    request->AddHandler(&request_handler_filereceive_);
    if (GetReceiverConfig()->kafka_config.enabled) {
        request->AddHandler(&request_handler_kafka_notify_);
    }
    return nullptr;
}

Error RequestFactory::AddHandlersToRequest(std::unique_ptr<Request>& request,
                                           const GenericRequestHeader& request_header) const {
    if (request_header.op_code != Opcode::kOpcodeAuthorize) {
        request->AddHandler(&request_handler_secondary_authorize_);
    }

    switch (request_header.op_code) {
    case Opcode::kOpcodeTransferData:
    case Opcode::kOpcodeTransferDatasetData: {
        request->AddHandler(&request_handler_receive_metadata_);
        auto err = AddReceiveWriteHandlers(request, request_header);
        if (err) {
            return err;
        }
        if (NeedDbHandler(request_header)) {
            request->AddHandler(&request_handler_dbwrite_);
        }
        break;
    }
    case Opcode::kOpcodeTransferMetaData: {
        if (NeedDbHandler(request_header)) {
            request->AddHandler(&request_handler_receivedata_);
            request->AddHandler(&request_handler_db_meta_write_);
        } else {
            return ReceiverErrorTemplates::kInternalServerError.Generate(
                       "reciever does not support writing to database");
        }
        break;
    }
    case Opcode::kOpcodeAuthorize: {
        request->AddHandler(&request_handler_receive_metadata_);
        request->AddHandler(&request_handler_initial_authorize_);
        break;
    }
    case Opcode::kOpcodeStreamInfo: {
        request->AddHandler(&request_handler_db_stream_info_);
        break;
    }
    case Opcode::kOpcodeDeleteStream: {
        request->AddHandler(&request_handler_delete_stream_);
        break;
    }
    case Opcode::kOpcodeLastStream: {
        request->AddHandler(&request_handler_db_last_stream_);
        break;
    }
    case Opcode::kOpcodeGetMeta: {
        request->AddHandler(&request_handler_db_get_meta_);
        break;
    }
    default:
        return ReceiverErrorTemplates::kInvalidOpCode.Generate();
    }

    return nullptr;

}

std::unique_ptr<Request> RequestFactory::GenerateRequest(const GenericRequestHeader&
        request_header, SocketDescriptor socket_fd, std::string origin_uri,
        Error* err) const noexcept {
    auto request = std::unique_ptr<Request> {new Request{request_header, socket_fd, std::move(origin_uri), cache_.get(),
                &request_handler_db_check_}
    };
    *err = AddHandlersToRequest(request, request_header);
    if (*err) {
        return nullptr;
    }
    return request;
}

RequestFactory::RequestFactory(SharedCache cache, KafkaClient* kafka_client) : request_handler_kafka_notify_{kafka_client}, cache_{cache} {
}

}
