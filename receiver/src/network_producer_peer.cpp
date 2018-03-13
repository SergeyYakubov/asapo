#include <cstring>
#include <assert.h>
#include "network_producer_peer.h"

namespace hidra2 {

size_t NetworkProducerPeer::kRequestHandlerMaxBufferSize;
std::atomic<uint32_t> NetworkProducerPeer::kNetworkProducerPeerCount;

const std::vector<NetworkProducerPeer::RequestHandlerInformation> NetworkProducerPeer::kRequestHandlers =
    NetworkProducerPeer::init_request_handlers();

NetworkProducerPeer::NetworkProducerPeer(int socket_fd, std::string address) : HasIO() {
    socket_fd_ = socket_fd;
    connection_id_ = kNetworkProducerPeerCount++;
}

uint32_t NetworkProducerPeer::GetConnectionId() const {
    return connection_id_;
}

void NetworkProducerPeer::start_peer_listener() {
    if(listener_thread_ || is_listening_)
        return;
    is_listening_ = true;
    listener_thread_ = io->NewThread([this] {
        internal_receiver_thread_();
    });
}

void NetworkProducerPeer::internal_receiver_thread_() {
    Error io_err;
    while(is_listening_) {
        io_err = nullptr;

        io->ReceiveWithTimeout(socket_fd_, generic_request.get(), sizeof(GenericNetworkRequest), 50, &io_err);
        if(io_err != nullptr) {
            if(IOErrorTemplates::kTimeout == io_err) {
                std::this_thread::yield();
                continue;
            }

            if(ErrorTemplates::kEndOfFile == io_err) {
                is_listening_ = false;
                break;
            }

            std::cerr << "[" << GetConnectionId() << "] Fail to receive data" << std::endl;
            is_listening_ = false;
            break;
        }

        std::cout << "[" << GetConnectionId() << "] Got request: " << generic_request->op_code << std::endl;

        //generic_response will be set here and the amount to send is returned
        size_t bytes_to_send = handle_generic_request_(generic_request.get(), generic_response.get());

        if(bytes_to_send == 0) {
            continue;//No data to send
        }

        io->Send(socket_fd_, generic_response.get(), bytes_to_send, &io_err);

        if(io_err != nullptr) {
            std::cerr << "[" << GetConnectionId() << "] Fail to send response" << std::endl;
        }
    }

    io->CloseSocket(socket_fd_, nullptr);
    std::cout << "[" << GetConnectionId() << "] Disconnected." << std::endl;
}

void NetworkProducerPeer::stop_peer_listener() {
    is_listening_ = false;
    if(!listener_thread_)
        return;
    listener_thread_->join();
    listener_thread_ = nullptr;
}

size_t NetworkProducerPeer::handle_generic_request_(GenericNetworkRequest* request, GenericNetworkResponse* response) {
    if(request->op_code >= kNetOpcodeCount || request->op_code < 0) {
        std::cerr << "[" << GetConnectionId() << "] Error invalid op_code: " << request->op_code << " force disconnect." <<
                  std::endl;
        io->CloseSocket(socket_fd_, nullptr);
        return 0;
    }

    response->request_id = request->request_id;
    response->op_code = request->op_code;

    auto handler_information = kRequestHandlers[request->op_code];

    Error io_err;

    static const size_t sizeof_generic_request = sizeof(GenericNetworkRequest);
    //after receiving all GenericNetworkResponse fields from caller,
    //we need now need to receive the rest of the request
    io->Receive(socket_fd_, (uint8_t*)request + sizeof_generic_request,
                handler_information.request_size - sizeof_generic_request, &io_err);

    if(io_err != nullptr) {
        std::cerr << "[" << GetConnectionId() << "] NetworkProducerPeer::handle_generic_request_/receive_timeout: " <<
                  request->op_code << std::endl;
        return 0;
    }

    handler_information.handler(this, request, response);

    return handler_information.response_size;
}

NetworkProducerPeer::~NetworkProducerPeer() {
    stop_peer_listener();
}

FileDescriptor NetworkProducerPeer::CreateAndOpenFileByFileId(uint64_t file_id, Error* err) {
    io->CreateNewDirectory("files", err);
    if(*err != nullptr && *err != IOErrorTemplates::kFileAlreadyExists) {
        return -1;
    }
    return io->Open("files/" + std::to_string(file_id) + ".bin", IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS | IO_OPEN_MODE_RW,
                    err);
}

bool NetworkProducerPeer::CheckIfValidFileSize(size_t file_size) {
    return file_size != 0 && file_size <= size_t(1024) * 1024 * 1024 * 2;

}

}

