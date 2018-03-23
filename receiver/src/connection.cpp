#include <cstring>
#include <assert.h>
#include "connection.h"
#include "receiver_error.h"
#include "system_wrappers/system_io.h"

namespace hidra2 {

size_t Connection::kRequestHandlerMaxBufferSize;
std::atomic<uint32_t> Connection::kNetworkProducerPeerImplGlobalCounter(0);

Connection::Connection(SocketDescriptor socket_fd, const std::string& address): request_factory__{new RequestFactory},
io__{new SystemIO} {
    socket_fd_ = socket_fd;
    connection_id_ = kNetworkProducerPeerImplGlobalCounter++;
    address_ = address;
}

uint64_t Connection::GetId() const noexcept {
    return connection_id_;
}

NetworkErrorCode GetNetworkCodeFromError(const Error& err) {
    if(err) {
        if(err == IOErrorTemplates::kFileAlreadyExists) {
            return NetworkErrorCode::kNetErrorFileIdAlreadyInUse;
        } else {
            return NetworkErrorCode::kNetErrorInternalServerError;
        }
    }
    return NetworkErrorCode::kNetErrorNoError;

}

Error Connection::ProcessRequest(const std::unique_ptr<Request>& request) const noexcept {
    Error err;
    err = request->Handle();
    GenericNetworkResponse generic_response;
    generic_response.error_code = GetNetworkCodeFromError(err);
    if(err) {
        std::cerr << "[" << GetId() << "] Error while handling request: " << err << std::endl;
    }
    io__->Send(socket_fd_, &generic_response, sizeof(GenericNetworkResponse), &err);
    return err;
}


void Connection::Listen() const noexcept {
    while(true) {
        Error err;
        auto request = WaitForNewRequest(&err);
        if(err) {
            std::cerr << "[" << GetId() << "] Error while waiting for request: " << err << std::endl;
            break;
        }
        if (!request) continue; //no error, but timeout
        err = ProcessRequest(request);
        if(err) {
            std::cerr << "[" << GetId() << "] Error sending response: " << err << std::endl;
            break;
        }
    }
    io__->CloseSocket(socket_fd_, nullptr);
    std::cout << "[" << GetId() << "] Disconnected." << std::endl;
}


std::unique_ptr<Request> Connection::WaitForNewRequest(Error* err) const noexcept {
    //TODO: to be overwritten with MessagePack (or similar)
    GenericNetworkRequestHeader generic_request_header;
    io__->ReceiveWithTimeout(socket_fd_, &generic_request_header, sizeof(GenericNetworkRequestHeader), 50, err);
    if(*err) {
        if(*err == IOErrorTemplates::kTimeout) {
            *err = nullptr;//Not an error in this case
        }
        return nullptr;
    }
    return request_factory__->GenerateRequest(generic_request_header, socket_fd_, err);
}


/*

void Connection::HandleRawRequestBuffer(GenericNetworkRequestHeader* request,
                                        GenericNetworkResponse* response,
                                        Error* err) noexcept {
    std::cout << "[" << GetId() << "] Got request op_code: " << request->op_code << std::endl;

    //response will be set here and the amount to send is returned
    size_t bytes_to_send = HandleGenericRequest(request, response, err);
    if(*err) {
        std::cerr << "[" << GetId() << "] Error occurred while handling op_code: " << request->op_code << std::endl;
        return;
    }

    if(bytes_to_send == 0) {
        return;//No data to send
    }

    io__->Send(socket_fd_, response, bytes_to_send, err);
}

size_t Connection::HandleGenericRequest(GenericNetworkRequestHeader* request,
                                        GenericNetworkResponse* response, Error* err) noexcept {
    if(!CheckIfValidNetworkOpCode(request->op_code)) {
        *err = hidra2::ReceiverErrorTemplates::kInvalidOpCode.Generate();
        return 0;
    }

    response->request_id = request->request_id;
    response->op_code = request->op_code;

    auto handler_information = kRequestHandlers[request->op_code];

    static const size_t sizeof_generic_request = sizeof(GenericNetworkRequestHeader);

    //after receiving all GenericNetworkResponse fields (did the caller already),
    //we need now need to receive the rest of the request
    io__->Receive(socket_fd_, (uint8_t*)request + sizeof_generic_request,
                handler_information.request_size - sizeof_generic_request, err);

    if(*err) {
        return 0;
    }

    //Invoke the request handler which sets the response
    handler_information.handler(this, request, response, err);

    if(*err) {
        return 0;
    }

    return handler_information.response_size;
}

Connection::~Connection() {
}

FileDescriptor Connection::CreateAndOpenFileByFileId(uint64_t data_id, Error* err) const noexcept {
    io__->CreateNewDirectory("files", err);
    if(*err && *err != IOErrorTemplates::kFileAlreadyExists) {
        return -1;
    }
    return io__->Open("files/" + std::to_string(data_id) + ".bin", IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS | IO_OPEN_MODE_RW,
                    err);
}

bool Connection::CheckIfValidFileSize(size_t data_size) const noexcept {
    return data_size != 0 && data_size <= size_t(1024) * 1024 * 1024 * 2;
}

bool Connection::CheckIfValidNetworkOpCode(Opcode opcode) const noexcept {
    return opcode < kNetOpcodeCount && opcode >= 0;
}

void Connection::ReceiveAndSaveFile(uint64_t data_id, size_t data_size, Error* err) const noexcept {
    if(!CheckIfValidFileSize(data_size)) {
        *err = ErrorTemplates::kMemoryAllocationError.Generate();
        return;
    }

    FileDescriptor fd = CreateAndOpenFileByFileId(data_id, err);
    if(*err) {
        if(*err != IOErrorTemplates::kFileAlreadyExists) {
            return; //Unexpected error
        }
        Error skipErr;//Create a new error instance so that the original error will not be overwritten
        io__->Skip(socket_fd_, data_size, &skipErr);//Skip the file payload so it will not get out of sync
        return;
    }

    FileData buffer;
    try {
        buffer.reset(new uint8_t[data_size]);
    } catch(std::exception& e) {
        *err = ErrorTemplates::kMemoryAllocationError.Generate();
        (*err)->Append(e.what());
        return;
    }

    io__->Receive(socket_fd_, buffer.get(), data_size, err);
    if(*err) {
        return;
    }

    io__->Write(fd, buffer.get(), data_size, err);
    if(*err) {
        return;
    }
}

const std::vector<Connection::RequestHandlerInformation>
Connection::StaticInitRequestHandlerList() {
    std::vector<Connection::RequestHandlerInformation> vec(kNetOpcodeCount);

    // Add new opcodes here
    vec[kNetOpcodeSendData] = {
        sizeof(GenericNetworkRequestHeader),
        sizeof(SendDataResponse),
        (Connection::RequestHandler)& Connection::HandleSendDataRequestInternalCaller
    };

    for(RequestHandlerInformation& handler_information : vec) {
        //Adjust max size needed for a request/response-buffer

        if(handler_information.request_size > kRequestHandlerMaxBufferSize) {
            kRequestHandlerMaxBufferSize = handler_information.request_size;
        }
        if(handler_information.response_size > kRequestHandlerMaxBufferSize) {
            kRequestHandlerMaxBufferSize = handler_information.response_size;
        }
    }

    return vec;
}


void Connection::HandleSendDataRequestInternalCaller(Connection* self,
        const GenericNetworkRequestHeader* request,
        SendDataResponse* response,
        Error* err) noexcept {
    self->HandleSendDataRequest(request, response, err);
}

void Connection::HandleSendDataRequest(const GenericNetworkRequestHeader* request, SendDataResponse* response,
                                       Error* err) noexcept {
    ReceiveAndSaveFile(request->data_id, request->data_size, err);

    if(!*err) {
        response->error_code = kNetErrorNoError;
        return;
    }

    if(*err == IOErrorTemplates::kFileAlreadyExists) {
        response->error_code = kNetErrorFileIdAlreadyInUse;
    } else {
        std::cout << "[" << GetId() << "] Unexpected ReceiveAndSaveFile error " << *err << std::endl;
        response->error_code = kNetErrorInternalServerError;
        //self->io__->CloseSocket(self->socket_fd_, nullptr); TODO: Might want to close the connection?
    }
}
*/

}

