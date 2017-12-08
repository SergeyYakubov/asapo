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

    fd_set read_fds;
    FD_SET(socket_fd_, &read_fds);
    timeval timeout;

    while (true) {
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int res = select(socket_fd_+1, &read_fds, nullptr, nullptr, &timeout);
        if(res == 0) {
            //timeout occurred
            continue;
        }
        if(res == -1) {
            std::cout << "[" << connection_id() << "] Error on select(): " << strerror(errno) << std::endl;
            break;
        }

        if(io_utils->recv_in_steps(socket_fd_, generic_request, sizeof(GenericNetworkRequest), 0) <= 0) {
            //Disconnect
            io->close(socket_fd_);
            std::cout << "[" << connection_id() << "] Disconnected." << std::endl;
            break;
        }

        std::cout << "[" << connection_id() << "] Got request: " << generic_request->op_code << std::endl;
        size_t bytes_to_send = handle_generic_request_(generic_request, generic_response);
        if(bytes_to_send == 0) {
            continue;
        }
        io_utils->send_in_steps(socket_fd_, generic_response, bytes_to_send, 0);
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

    close(socket_fd_);

    std::cout << "[" << connection_id() << "] Disconnected." << std::endl;
}

size_t NetworkProducerPeer::handle_generic_request_(GenericNetworkRequest* request, GenericNetworkResponse* response) {
    if(request->op_code >= OP_CODE_COUNT || request->op_code < 0) {
        std::cerr << "[" << connection_id() << "] Error invalid op_code: " << request->op_code << std::endl;
        close(socket_fd_);
        return 0;
    }

    response->request_id = request->request_id;
    response->op_code = request->op_code;

    auto handler_information = kRequestHandlers[request->op_code];

    //receive the rest of the message
    io_utils->recv_in_steps(socket_fd_, request->data, handler_information.request_size - sizeof(GenericNetworkRequest), 0);

    handler_information.handler(this, request, response);

    return handler_information.response_size;
}

}

