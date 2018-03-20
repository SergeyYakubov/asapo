#include <cstring>
#include <assert.h>
#include "connection.h"
#include "receiver_error.h"

namespace hidra2 {

size_t Connection::kRequestHandlerMaxBufferSize;
std::atomic<uint32_t> Connection::kNetworkProducerPeerImplGlobalCounter(0);

const std::vector<Connection::RequestHandlerInformation> Connection::kRequestHandlers =
    Connection::StaticInitRequestHandlerList();

Connection::Connection(SocketDescriptor socket_fd, const std::string& address) {
    socket_fd_ = socket_fd;
    connection_id_ = kNetworkProducerPeerImplGlobalCounter++;
    address_ = address;
}

uint32_t Connection::GetId() const {
    return connection_id_;
}

std::string Connection::GetAddress() const {
    return address_;
}

void Connection::Listen() noexcept {

    std::unique_ptr<GenericNetworkRequest> generic_request_buffer;
    std::unique_ptr<GenericNetworkResponse> generic_response_buffer;
    try {
        generic_request_buffer.reset(reinterpret_cast<GenericNetworkRequest*> (new uint8_t[kRequestHandlerMaxBufferSize]));
        generic_response_buffer.reset(reinterpret_cast<GenericNetworkResponse*> (new uint8_t[kRequestHandlerMaxBufferSize]));
    } catch(...) {
        std::cerr << "Failed to allocate buffer space for request and response" << std::endl;
        return;
    }

    Error err;
    while(true) {
        err = nullptr;
        ProcessRequestFromProducer(generic_request_buffer.get(), generic_response_buffer.get(), &err);
        if(err) {
            std::cerr << "[" << GetId() << "] Error while handling request: " << err << std::endl;
            break;
        }
    }

    io->CloseSocket(socket_fd_, nullptr);
    std::cout << "[" << GetId() << "] Disconnected." << std::endl;
}


void Connection::ProcessRequestFromProducer(GenericNetworkRequest* request,
                                            GenericNetworkResponse* response,
                                            Error* err) noexcept {
    io->ReceiveWithTimeout(socket_fd_, request, sizeof(GenericNetworkRequest), 50, err);
    if(*err) {
        if(*err == IOErrorTemplates::kTimeout) {
            *err = nullptr;//Not an error in this case
        }
        return;
    }
    HandleRawRequestBuffer(request, response, err);
}


void Connection::HandleRawRequestBuffer(GenericNetworkRequest* request,
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

    io->Send(socket_fd_, response, bytes_to_send, err);
}

size_t Connection::HandleGenericRequest(GenericNetworkRequest* request,
                                        GenericNetworkResponse* response, Error* err) noexcept {
    if(!CheckIfValidNetworkOpCode(request->op_code)) {
        *err = hidra2::ReceiverErrorTemplates::kInvalidOpCode.Generate();
        return 0;
    }

    response->request_id = request->request_id;
    response->op_code = request->op_code;

    auto handler_information = kRequestHandlers[request->op_code];

    static const size_t sizeof_generic_request = sizeof(GenericNetworkRequest);

    //after receiving all GenericNetworkResponse fields (did the caller already),
    //we need now need to receive the rest of the request
    io->Receive(socket_fd_, (uint8_t*)request + sizeof_generic_request,
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

FileDescriptor Connection::CreateAndOpenFileByFileId(uint64_t file_id, Error* err) const noexcept {
    io->CreateNewDirectory("files", err);
    if(*err && *err != IOErrorTemplates::kFileAlreadyExists) {
        return -1;
    }
    return io->Open("files/" + std::to_string(file_id) + ".bin", IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS | IO_OPEN_MODE_RW,
                    err);
}

bool Connection::CheckIfValidFileSize(size_t file_size) const noexcept {
    return file_size != 0 && file_size <= size_t(1024) * 1024 * 1024 * 2;
}

bool Connection::CheckIfValidNetworkOpCode(Opcode opcode) const noexcept {
    return opcode < kNetOpcodeCount && opcode >= 0;
}

void Connection::ReceiveAndSaveFile(uint64_t file_id, size_t file_size, Error* err) const noexcept {
    if(!CheckIfValidFileSize(file_size)) {
        *err = ErrorTemplates::kMemoryAllocationError.Generate();
        return;
    }

    FileDescriptor fd = CreateAndOpenFileByFileId(file_id, err);
    if(*err) {
        if(*err != IOErrorTemplates::kFileAlreadyExists) {
            return; //Unexpected error
        }
        Error skipErr;//Create a new error instance so that the original error will not be overwritten
        io->Skip(socket_fd_, file_size, &skipErr);//Skip the file payload so it will not get out of sync
        return;
    }

    FileData buffer;
    try {
        buffer.reset(new uint8_t[file_size]);
    } catch(std::exception& e) {
        *err = ErrorTemplates::kMemoryAllocationError.Generate();
        (*err)->Append(e.what());
        return;
    }

    io->Receive(socket_fd_, buffer.get(), file_size, err);
    if(*err) {
        return;
    }

    io->Write(fd, buffer.get(), file_size, err);
    if(*err) {
        return;
    }
}

const std::vector<Connection::RequestHandlerInformation>
Connection::StaticInitRequestHandlerList() {
    std::vector<Connection::RequestHandlerInformation> vec(kNetOpcodeCount);

    // Add new opcodes here
    vec[kNetOpcodeSendData] = {
        sizeof(SendDataRequest),
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
        const SendDataRequest* request,
        SendDataResponse* response,
        Error* err) noexcept {
    self->HandleSendDataRequest(request, response, err);
}

void Connection::HandleSendDataRequest(const SendDataRequest* request, SendDataResponse* response,
                                       Error* err) noexcept {
    ReceiveAndSaveFile(request->file_id, request->file_size, err);

    if(!*err) {
        response->error_code = kNetErrorNoError;
        return;
    }

    if(*err == IOErrorTemplates::kFileAlreadyExists) {
        response->error_code = kNetErrorFileIdAlreadyInUse;
    } else {
        std::cout << "[" << GetId() << "] Unexpected ReceiveAndSaveFile error " << *err << std::endl;
        response->error_code = kNetErrorInternalServerError;
        //self->io->CloseSocket(self->socket_fd_, nullptr); TODO: Might want to close the connection?
    }
}


}

