#include "request_factory.h"

#include "receiver_config.h"

namespace asapo {

bool NeedFileWriteHandler (const GenericRequestHeader& request_header) {
    return GetReceiverConfig()->write_to_disk &&
           (request_header.custom_data[kPosIngestMode] & IngestModeFlags::kStoreInFilesystem);
}

bool NeedDbHandler (const GenericRequestHeader& request_header) {
    return GetReceiverConfig()->write_to_db;
}

Error RequestFactory::AddReceiveWriteHandlers(std::unique_ptr<Request>& request,const GenericRequestHeader& request_header) const  {
    request->AddHandler(&request_handler_receivedata_);
    if (NeedFileWriteHandler(request_header)) {
        request->AddHandler(&request_handler_filewrite_);
    }
    return nullptr;
}

Error RequestFactory::AddHandlersToRequest(std::unique_ptr<Request>& request,
                                           const GenericRequestHeader& request_header) const {
    request->AddHandler(&request_handler_authorize_);

    switch (request_header.op_code) {
    case Opcode::kOpcodeTransferData:
    case Opcode::kOpcodeTransferSubsetData:        {
        AddReceiveWriteHandlers(request,request_header);
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
            return ReceiverErrorTemplates::kReject.Generate("reciever does not support writing to database");
        }
        break;
    }
    case Opcode::kOpcodeAuthorize: {
        // do nothing
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
    auto request = std::unique_ptr<Request> {new Request{request_header, socket_fd, std::move(origin_uri), cache_.get()}};
    *err = AddHandlersToRequest(request, request_header);
    if (*err) {
        return nullptr;
    }
    return request;
}


RequestFactory::RequestFactory(SharedCache cache): cache_{cache} {

}


}