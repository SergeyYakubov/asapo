#include <cstring>
#include <assert.h>
#include "network_producer_peer.h"

namespace hidra2 {

const size_t NetworkProducerPeer::kGenericBufferSize = 1024 * 50; //50KiByte
std::atomic<uint32_t> NetworkProducerPeer::kNetworkProducerPeerCount;

const std::vector<NetworkProducerPeer::RequestHandlerInformation> NetworkProducerPeer::kRequestHandlers =
    NetworkProducerPeer::init_request_handlers();

NetworkProducerPeer::NetworkProducerPeer(int socket_fd, std::string address) : HasIO() {
    address_ = std::move(address);
    socket_fd_ = socket_fd;
    connection_id_ = kNetworkProducerPeerCount++;
}

uint32_t NetworkProducerPeer::connection_id() const {
    return connection_id_;
}

const std::string& NetworkProducerPeer::address() const {
    return address_;
}

bool NetworkProducerPeer::is_listening() const {
    return is_listening_;
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
    auto* const generic_request = (GenericNetworkRequest*) malloc(kGenericBufferSize);
    auto* const generic_response = (GenericNetworkResponse*) malloc(kGenericBufferSize);

    IOErrors err;
    while(is_listening_) {
        err = IOErrors::kNoError;

        io->ReceiveTimeout(socket_fd_, generic_request, sizeof(GenericNetworkRequest), 1, &err);

        if(err != IOErrors::kNoError) {
            if(err == IOErrors::kTimeout) {
                std::this_thread::yield();
                continue;
            }

            if(err == IOErrors::kEndOfFile) {
                is_listening_ = false;
                break;
            }

            std::cerr << "[" << connection_id() << "] Fail to receive data" << std::endl;
            is_listening_ = false;
            break;
        }

        std::cout << "[" << connection_id() << "] Got request: " << generic_request->op_code << std::endl;
        size_t bytes_to_send = handle_generic_request_(generic_request, generic_response);

        if(bytes_to_send == 0) {
            continue;//No data to send
        }

        io->Send(socket_fd_, generic_response, bytes_to_send, &err);

        if(err != IOErrors::kNoError) {
            std::cerr << "[" << connection_id() << "] Fail to send response" << std::endl;
        }
    }

    io->Close(socket_fd_, nullptr);
    std::cout << "[" << connection_id() << "] Disconnected." << std::endl;

    free(generic_request);
    free(generic_response);

}

void NetworkProducerPeer::stop_peer_listener() {
    is_listening_ = false;
    if(!listener_thread_)
        return;
    listener_thread_->join();
    delete listener_thread_;
    listener_thread_ = nullptr;
}

size_t NetworkProducerPeer::handle_generic_request_(GenericNetworkRequest* request, GenericNetworkResponse* response) {
    if(request->op_code >= OP_CODE_COUNT || request->op_code < 0) {
        std::cerr << "[" << connection_id() << "] Error invalid op_code: " << request->op_code << " force disconnect." <<
                  std::endl;
        io->Close(socket_fd_, nullptr);
        return 0;
    }

    response->request_id = request->request_id;
    response->op_code = request->op_code;

    auto handler_information = kRequestHandlers[request->op_code];

    assert(handler_information.request_size <= kGenericBufferSize);//Would overwrite memory
    assert(handler_information.response_size <= kGenericBufferSize);//Would overwrite memory

    IOErrors err;
    //receive the rest of the message
    io->Receive(socket_fd_, request + sizeof(GenericNetworkRequest),
                handler_information.request_size - sizeof(GenericNetworkRequest), &err);
    if(err != IOErrors::kNoError) {
        std::cerr << "[" << connection_id() << "] NetworkProducerPeer::handle_generic_request_/receive_timeout: " <<
                  request->op_code << std::endl;
        return 0;
    }

    handler_information.handler(this, request, response);

    return handler_information.response_size;
}

NetworkProducerPeer::~NetworkProducerPeer() {
    stop_peer_listener();
}

FileDescriptor NetworkProducerPeer::CreateAndOpenFileByFileId(uint64_t file_id, IOErrors* err) {
    io->CreateDirectory("files", err);
    if(*err != IOErrors::kNoError && *err != IOErrors::kFileAlreadyExists) {
        return -1;
    }
    return io->Open("files/" + std::to_string(file_id) + ".bin", IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS | IO_OPEN_MODE_RW,
                    err);
}


}

