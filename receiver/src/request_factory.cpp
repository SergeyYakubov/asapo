#include "request_factory.h"

#include "receiver_config.h"

namespace asapo {

std::unique_ptr<Request> RequestFactory::GenerateRequest(const GenericRequestHeader&
        request_header, SocketDescriptor socket_fd, std::string origin_uri,
        Error* err) const noexcept {
    *err = nullptr;
    auto request = std::unique_ptr<Request> {new Request{request_header, socket_fd, std::move(origin_uri), cache_.get()}};
    switch (request_header.op_code) {
    case Opcode::kOpcodeTransferData:
    case Opcode::kOpcodeTransferSubsetData:        {
        request->AddHandler(&request_handler_authorize_);
        if (GetReceiverConfig()->write_to_disk) {
            request->AddHandler(&request_handler_filewrite_);
        }
        if (GetReceiverConfig()->write_to_db) {
            request->AddHandler(&request_handler_dbwrite_);
        }
        return request;
    }
    case Opcode::kOpcodeTransferMetaData: {
        request->AddHandler(&request_handler_authorize_);
        if (GetReceiverConfig()->write_to_db) {
            request->AddHandler(&request_handler_db_meta_write_);
        } else {
            *err = ReceiverErrorTemplates::kReject.Generate("reciever does not support writing to database");
            return nullptr;
        }
        return request;
    }
    case Opcode::kOpcodeAuthorize: {
        request->AddHandler(&request_handler_authorize_);
        return request;
    }
    default:
        *err = ReceiverErrorTemplates::kInvalidOpCode.Generate();
        return nullptr;
    }
}
RequestFactory::RequestFactory(SharedCache cache): cache_{cache} {

}


}