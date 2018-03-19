#include <cstring>
#include <assert.h>
#include "network_producer_peer_impl.h"
#include "receiver_error.h"

namespace hidra2 {

size_t NetworkProducerPeerImpl::kRequestHandlerMaxBufferSize;
std::atomic<uint32_t> NetworkProducerPeerImpl::kNetworkProducerPeerImplGlobalCounter(0);

const std::vector<NetworkProducerPeerImpl::RequestHandlerInformation> NetworkProducerPeerImpl::kRequestHandlers =
    NetworkProducerPeerImpl::StaticInitRequestHandlerList();

NetworkProducerPeerImpl::NetworkProducerPeerImpl(SocketDescriptor socket_fd, const std::string& address) {
    socket_fd_ = socket_fd;
    connection_id_ = kNetworkProducerPeerImplGlobalCounter++;
    address_ = address;
}

uint32_t NetworkProducerPeerImpl::GetConnectionId() const {
    return connection_id_;
}

std::string NetworkProducerPeerImpl::GetAddress() const {
    return address_;
}

void NetworkProducerPeerImpl::StartPeerListener() {
    if(listener_thread_ || is_listening_)
        return;
    is_listening_ = true;
    listener_thread_ = io->NewThread([this] {
        InternalPeerReceiverThreadEntryPoint();
    });
}

void NetworkProducerPeerImpl::InternalPeerReceiverThreadEntryPoint() noexcept {

    std::unique_ptr<GenericNetworkRequest> generic_request_buffer;
    std::unique_ptr<GenericNetworkResponse> generic_response_buffer;
    try {
        generic_request_buffer.reset(reinterpret_cast<GenericNetworkRequest*> (new uint8_t[kRequestHandlerMaxBufferSize]));
        generic_response_buffer.reset(reinterpret_cast<GenericNetworkResponse*> (new uint8_t[kRequestHandlerMaxBufferSize]));
    } catch(...) {
        std::cerr << "Failed to allocate buffer space for request and response" << std::endl;
        is_listening_ = false;
    }

    Error err;
    while(is_listening_) {
        err = nullptr;
        InternalPeerReceiverDoWork(generic_request_buffer.get(), generic_response_buffer.get(), &err);
        if(err) {
            std::cerr << "[" << GetConnectionId() << "] Error while handling work: " << err << std::endl;
            is_listening_ = false;
        }
    }

    io->CloseSocket(socket_fd_, nullptr);
    std::cout << "[" << GetConnectionId() << "] Disconnected." << std::endl;
}


void NetworkProducerPeerImpl::InternalPeerReceiverDoWork(GenericNetworkRequest* request,
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


void NetworkProducerPeerImpl::HandleRawRequestBuffer(GenericNetworkRequest* request,
        GenericNetworkResponse* response,
        Error* err) noexcept {
    std::cout << "[" << GetConnectionId() << "] Got request op_code: " << request->op_code << std::endl;

    //response will be set here and the amount to send is returned
    size_t bytes_to_send = HandleGenericRequest(request, response, err);
    if(*err) {
        std::cerr << "[" << GetConnectionId() << "] Error occurred while handling op_code: " << request->op_code << std::endl;
        return;
    }

    if(bytes_to_send == 0) {
        return;//No data to send
    }

    io->Send(socket_fd_, response, bytes_to_send, err);
}

void NetworkProducerPeerImpl::StopPeerListener() {
//    is_listening_ = false;
    if(!listener_thread_)
        return;
    listener_thread_->join();
    listener_thread_ = nullptr;
}

size_t NetworkProducerPeerImpl::HandleGenericRequest(GenericNetworkRequest* request,
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

NetworkProducerPeerImpl::~NetworkProducerPeerImpl() {
    StopPeerListener();
}

FileDescriptor NetworkProducerPeerImpl::CreateAndOpenFileByFileId(uint64_t file_id, Error* err) const noexcept {
    io->CreateNewDirectory("files", err);
    if(*err && *err != IOErrorTemplates::kFileAlreadyExists) {
        return -1;
    }
    return io->Open("files/" + std::to_string(file_id) + ".bin", IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS | IO_OPEN_MODE_RW,
                    err);
}

bool NetworkProducerPeerImpl::CheckIfValidFileSize(size_t file_size) const noexcept {
    return file_size != 0 && file_size <= size_t(1024) * 1024 * 1024 * 2;
}

bool NetworkProducerPeerImpl::CheckIfValidNetworkOpCode(Opcode opcode) const noexcept {
    return opcode < kNetOpcodeCount && opcode >= 0;
}


}

