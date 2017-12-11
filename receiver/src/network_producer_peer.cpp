#include <cstring>
#include "network_producer_peer.h"

namespace hidra2 {

FileReferenceHandler NetworkProducerPeer::file_reference_handler;
std::atomic<uint32_t> NetworkProducerPeer::kNetworkProducerPeerCount;

const std::vector<NetworkProducerPeer::RequestHandlerInformation> NetworkProducerPeer::kRequestHandlers = NetworkProducerPeer::init_request_handlers();

NetworkProducerPeer::NetworkProducerPeer(int socket_fd, std::string address)  {
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

void NetworkProducerPeer::start_peer_receiver() {
    if(receiver_thread_)
        return;
    receiver_thread_ = new std::thread([this] {
        internal_receiver_thread_();
    });
}

void NetworkProducerPeer::internal_receiver_thread_() {
    /** Must be as large as the largest request type (not including the data) */
    auto* const generic_request = (GenericNetworkRequest*) malloc(1024*50);
    auto* const generic_response = (GenericNetworkResponse*) malloc(1024*50);

    IOErrors err;
    while(true) {
        err = IOErrors::NO_ERROR;

        io->receive_timeout(socket_fd_, generic_request, sizeof(GenericNetworkRequest), 1, &err);

        if(err != IOErrors::NO_ERROR) {
            if(err == IOErrors::TIMEOUT) {
                continue;
            }

            if(err == IOErrors::STREAM_EOF) {
                io->deprecated_close(socket_fd_);
                std::cout << "[" << connection_id() << "] Disconnected." << std::endl;
                break;
            }
            std::cerr << "[" << connection_id() << "] Fail to receive data" << std::endl;
            break;
        }

        std::cout << "[" << connection_id() << "] Got request: " << generic_request->op_code << std::endl;
        size_t bytes_to_send = handle_generic_request_(generic_request, generic_response);

        if(bytes_to_send == 0) {
            continue;//No data to send
        }

        io->send(socket_fd_, generic_response, bytes_to_send, &err);

        if(err != IOErrors::NO_ERROR) {
            std::cerr << "[" << connection_id() << "] Fail to send response" << std::endl;
        }
    }

    free(generic_request);
    free(generic_response);

}

void NetworkProducerPeer::stop_peer_receiver() {
    if(!receiver_thread_)
        return;
    receiver_thread_->join();
    receiver_thread_ = nullptr;
}

void NetworkProducerPeer::disconnect() {
    stop_peer_receiver();

    io->deprecated_close(socket_fd_);

    std::cout << "[" << connection_id() << "] Disconnected." << std::endl;
}

size_t NetworkProducerPeer::handle_generic_request_(GenericNetworkRequest* request, GenericNetworkResponse* response) {
    if(request->op_code >= OP_CODE_COUNT || request->op_code < 0) {
        std::cerr << "[" << connection_id() << "] Error invalid op_code: " << request->op_code << std::endl;
        io->deprecated_close(socket_fd_);
        return 0;
    }

    response->request_id = request->request_id;
    response->op_code = request->op_code;

    auto handler_information = kRequestHandlers[request->op_code];

    IOErrors err;
    //receive the rest of the message
    io->receive_timeout(socket_fd_, request->data, handler_information.request_size - sizeof(GenericNetworkRequest), 30, &err);
    if(err != IOErrors::NO_ERROR) {
        std::cerr << "[" << connection_id() << "] NetworkProducerPeer::handle_generic_request_/receive_timeout: " << request->op_code << std::endl;
        return 0;
    }

    handler_information.handler(this, request, response);

    return handler_information.response_size;
}

}

