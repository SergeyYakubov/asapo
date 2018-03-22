#include "request.h"
#include "send_data_request.h"
#include "system_wrappers/system_io.h"

namespace hidra2 {

Request::Request(const std::unique_ptr<GenericNetworkRequestHeader>& header,
                 SocketDescriptor socket_fd) : io__{new SystemIO}, request_header_{*header}, socket_fd_{socket_fd} {
}

Error Request::Handle() {
    Error err;

    if (request_header_.data_size != 0) {
        try {
            data_buffer_.reset(new uint8_t[request_header_.data_size]);
        } catch(std::exception& e) {
            err = ErrorTemplates::kMemoryAllocationError.Generate();
            err->Append(e.what());
            return err;
        }

        io__->Receive(socket_fd_, data_buffer_.get(), request_header_.data_size, &err);
        if (err) {
            auto recv_err = ReceiverErrorTemplates::kConnectionError.Generate();
            recv_err->Append(err->Explain());
            return recv_err;
        }

    }
    return nullptr;
}

std::unique_ptr<Request> RequestFactory::GenerateRequest(const std::unique_ptr<GenericNetworkRequestHeader>&
        request_header, SocketDescriptor socket_fd,
        Error* err) const noexcept {
    *err = nullptr;
    switch (request_header->op_code) {
    case Opcode::kNetOpcodeSendData:
        return std::unique_ptr<Request> {new SendDataRequest{request_header, socket_fd}};
    default:
        *err = ReceiverErrorTemplates::kInvalidOpCode.Generate();
        return nullptr;
    }

}

}