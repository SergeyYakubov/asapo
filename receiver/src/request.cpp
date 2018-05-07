#include "request.h"
#include "io/io_factory.h"

#include "receiver_config.h"
namespace asapo {

Request::Request(const GenericNetworkRequestHeader& header,
                 SocketDescriptor socket_fd) : io__{GenerateDefaultIO()}, request_header_(header), socket_fd_{socket_fd} {
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


Error Request::Handle(std::unique_ptr<Statistics>* statistics) {
    Error err;
    if (request_header_.data_size != 0) {
        (*statistics)->StartTimer(StatisticEntity::kNetwork);
        auto err = ReceiveData();
        if (err) {
            return err;
        }
        (*statistics)->StopTimer();
    }
    for (auto handler : handlers_) {
        (*statistics)->StartTimer(handler->GetStatisticEntity());
        auto err = handler->ProcessRequest(*this);
        if (err) {
            return err;
        }
        (*statistics)->StopTimer();
    }
    return nullptr;
}

const RequestHandlerList& Request::GetListHandlers() const {
    return handlers_;
}


void Request::AddHandler(const RequestHandler* handler) {
    handlers_.emplace_back(handler);
}


uint64_t Request::GetDataID() const {
    return request_header_.data_id;
}


uint64_t Request::GetDataSize() const {
    return request_header_.data_size;
}

const FileData& Request::GetData() const {
    return data_buffer_;
}

std::string Request::GetFileName() const {
    return std::to_string(request_header_.data_id) + ".bin";
}

std::unique_ptr<Request> RequestFactory::GenerateRequest(const GenericNetworkRequestHeader&
        request_header, SocketDescriptor socket_fd,
        Error* err) const noexcept {
    *err = nullptr;
    switch (request_header.op_code) {
    case Opcode::kNetOpcodeSendData: {
        auto request = std::unique_ptr<Request> {new Request{request_header, socket_fd}};

        if (GetReceiverConfig()->write_to_disk) {
            request->AddHandler(&request_handler_filewrite_);
        }

        if (GetReceiverConfig()->write_to_db) {
            request->AddHandler(&request_handler_dbwrite_);
        }

        return request;
    }
    default:
        *err = ReceiverErrorTemplates::kInvalidOpCode.Generate();
        return nullptr;
    }
}


}