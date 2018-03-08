#include "network_producer_peer.h"
#include "receiver.h"
#include <cmath>

namespace hidra2 {

const std::vector<NetworkProducerPeer::RequestHandlerInformation> NetworkProducerPeer::init_request_handlers() {
    std::vector<NetworkProducerPeer::RequestHandlerInformation> vec(OP_CODE_COUNT);

    vec[OP_CODE__SEND_DATA] = {
        sizeof(SendDataRequest),
        sizeof(SendDataResponse),
        (NetworkProducerPeer::RequestHandler)& NetworkProducerPeer::handle_send_data_request_
    };

    return vec;
}


void NetworkProducerPeer::handle_send_data_request_(NetworkProducerPeer* self, const SendDataRequest* request,
        SendDataResponse* response) {
    Error io_err;

    if (request->file_size == 0) {
        std::cerr << "[" << self->connection_id() << "] file_id: " << request->file_id << " has size of 0!" << std::endl;
        response->error_code = NET_ERR__ALLOCATE_STORAGE_FAILED;
        return;
    }

    if(request->file_size > size_t(1024)*size_t(1024)*size_t(1024)*size_t(2)/*2GiByte*/) {
        response->error_code = NET_ERR__ALLOCATE_STORAGE_FAILED;
        return;
    }

    FileDescriptor fd = self->CreateAndOpenFileByFileId(request->file_id, &io_err);
    if(io_err != nullptr) {
        response->error_code = NET_ERR__FILEID_ALREADY_IN_USE;
        std::cerr << "[" << self->connection_id() << "] file_id: " << request->file_id << " does already exists" << std::endl;
        self->io->Skip(self->socket_fd_, request->file_size, &io_err);
        if(io_err != nullptr) {
            std::cout << "[" << self->connection_id() << "] Out of sync force disconnect" << std::endl;
            self->io->CloseSocket(self->socket_fd_, nullptr);
        }
        return;
    }

    std::unique_ptr<uint8_t[]> buffer;
    try {
        buffer.reset(new uint8_t[request->file_size]);
    } catch(std::exception& e) {
        std::cerr << "[" << self->connection_id() << "] Failed to allocate enough memory. file_id: " << request->file_id <<
                  " file_size:" << request->file_size << std::endl;
        std::cerr << e.what() << std::endl;
        response->error_code = NET_ERR__INTERNAL_SERVER_ERROR;
        return;
    }

    self->io->Receive(self->socket_fd_, buffer.get(), request->file_size, &io_err);
    if(io_err != nullptr) {
        std::cerr << "[" << self->connection_id() << "] An IO error occurred while receiving the file" << std::endl;
        response->error_code = NET_ERR__INTERNAL_SERVER_ERROR;
        return;
    }

    self->io->Write(fd, buffer.get(), request->file_size, &io_err);
    if(io_err != nullptr) {
        std::cerr << "[" << self->connection_id() << "] An IO error occurred while writing the file" << std::endl;
        response->error_code = NET_ERR__INTERNAL_SERVER_ERROR;
        return;
    }

    response->error_code = NET_ERR__NO_ERROR;
}

}

