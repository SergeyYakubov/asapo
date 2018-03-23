#include "request.h"
#include "system_wrappers/system_io.h"

namespace hidra2 {

Request::Request(const GenericNetworkRequestHeader& header,
                 SocketDescriptor socket_fd) : io__{new SystemIO}, request_header_{header}, socket_fd_{socket_fd} {
}

Error Request::AllocateDataBuffer() {
    try {
        data_buffer_.reset(new uint8_t[request_header_.data_size]);
    } catch(std::exception& e) {
        auto err = ErrorTemplates::kMemoryAllocationError.Generate();
        err->Append(e.what());
        return err;
    }
    return nullptr;
}

Error Request::ReceiveData() {
    auto err = AllocateDataBuffer();
    if (err) {
        return err;
    }
    io__->Receive(socket_fd_, data_buffer_.get(), request_header_.data_size, &err);
    return err;
}


Error Request::Handle() {
    Error err;
    if (request_header_.data_size != 0) {
        auto err = ReceiveData();
        if (err) {
            return err;
        }
    }
    for (auto handler : handlers_) {
        auto err = handler->ProcessRequest(*this);
        if (err) {
            return err;
        }
    }
    return nullptr;
}

const RequestHandlerList& Request::GetListHandlers() const {
    return handlers_;
}


void Request::AddHandler(const RequestHandler* handler) {
    handlers_.emplace_back(handler);
}


std::unique_ptr<Request> RequestFactory::GenerateRequest(const GenericNetworkRequestHeader&
        request_header, SocketDescriptor socket_fd,
        Error* err) const noexcept {
    *err = nullptr;
    switch (request_header.op_code) {
    case Opcode::kNetOpcodeSendData: {
        auto request = std::unique_ptr<Request> {new Request{request_header, socket_fd}};
        request->AddHandler(&request_handler_filewrite_);
        return request;
    }
    default:
        *err = ReceiverErrorTemplates::kInvalidOpCode.Generate();
        return nullptr;
    }

}

}