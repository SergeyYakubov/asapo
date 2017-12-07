#include "network_producer_peer.h"
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cmath>

namespace hidra2 {

const std::vector<NetworkProducerPeer::RequestHandlerInformation> NetworkProducerPeer::init_request_handlers() {
    std::vector<NetworkProducerPeer::RequestHandlerInformation> vec(OP_CODE_COUNT);

    vec[OP_CODE__HELLO] = {
        sizeof(HelloRequest),
        sizeof(HelloResponse),
        (NetworkProducerPeer::RequestHandler) &NetworkProducerPeer::handle_hello_request_
    };

    vec[OP_CODE__PREPARE_SEND_DATA] = {
        sizeof(PrepareSendDataRequest),
        sizeof(PrepareSendDataResponse),
        (NetworkProducerPeer::RequestHandler) &NetworkProducerPeer::handle_prepare_send_data_request_
    };

    vec[OP_CODE__SEND_DATA_CHUNK] = {
        sizeof(SendDataChunkRequest),
        sizeof(SendDataChunkResponse),
        (NetworkProducerPeer::RequestHandler) &NetworkProducerPeer::handle_send_data_chunk_request_
    };

    return vec;
}


void NetworkProducerPeer::handle_hello_request_(NetworkProducerPeer* self, const HelloRequest* request,
                                                HelloResponse* response) {
    std::cout << "op_code " << request->op_code << std::endl;
    std::cout << "request_id " << request->request_id << std::endl;

    std::cout << "client_version " << request->client_version << std::endl;
    std::cout << "os " << request->os << std::endl;
    std::cout << "is_x64 " << request->is_x64 << std::endl;

    response->error_code = NET_ERR__NO_ERROR;
    response->server_version = 1;
}

void NetworkProducerPeer::handle_prepare_send_data_request_(NetworkProducerPeer* self, const PrepareSendDataRequest* request,
                                                            PrepareSendDataResponse* response) {
    std::cout << "[PRE]op_code " << request->op_code << std::endl;
    std::cout << "[PRE]request_id " << request->request_id << std::endl;

    std::cout << "[PRE]filename " << request->filename << std::endl;
    std::cout << "[PRE]file_size " << request->file_size << std::endl;

    FileReferenceHandlerError error;
    FileReferenceId reference_id = self->file_reference_handler.add_file(request->filename,
                                                                         request->file_size,
                                                                         self->connection_id(),
                                                                         error);

    if(reference_id == 0 || error) {
        response->error_code = NET_ERR__INTERNAL_SERVER_ERROR;
        response->file_reference_id = 0;
        return;
    }

    response->error_code = NET_ERR__NO_ERROR;
    response->file_reference_id = reference_id;
}

void NetworkProducerPeer::handle_send_data_chunk_request_(NetworkProducerPeer* self,
                                                          const SendDataChunkRequest* request,
                                                          SendDataChunkResponse* response) {
    std::cout << "[CHUNK]op_code " << request->op_code << std::endl;
    std::cout << "[CHUNK]request_id " << request->request_id << std::endl;

    std::cout << "[CHUNK]file_reference_id " << request->file_reference_id << std::endl;
    std::cout << "[CHUNK]start_byte " << request->start_byte << std::endl;
    std::cout << "[CHUNK]chunk_size " << request->chunk_size << std::endl;

    auto file_info = self->file_reference_handler.get_file(request->file_reference_id);

    if(file_info == nullptr || file_info->owner != self->connection_id()) {
        response->error_code = NET_ERR__UNKNOWN_REFERENCE_ID;
        return;
    }

    size_t map_size = static_cast<size_t>(ceil(float(request->chunk_size)/float(getpagesize()))*getpagesize());

    void* mapped_file = mmap(nullptr,
                             map_size,
                             PROT_READ | PROT_WRITE, MAP_SHARED,
                             file_info->fd,
                             request->start_byte);

    if(!mapped_file || mapped_file == MAP_FAILED) {
        std::cerr << "Mapping a file faild" << std::endl;//TODO need to read to rest of the file into void
        self->io->recv(self->socket_fd_, nullptr, request->chunk_size, 0);
        response->error_code = NET_ERR__INTERNAL_SERVER_ERROR;
        return;
    }

    if(self->io->recv(self->socket_fd_, mapped_file, request->chunk_size, 0) != request->chunk_size) {
        std::cerr << "Fail to recv all the chunk data" << std::endl;
        response->error_code = NET_ERR__INTERNAL_SERVER_ERROR;
    }

    if (msync(mapped_file, map_size, MS_SYNC) == -1) {
        std::cerr << "Fail to sync map file" << std::endl;
        response->error_code = NET_ERR__INTERNAL_SERVER_ERROR;
    }

    if(munmap(mapped_file, map_size) == -1) {
        std::cerr << "munmap file faild" << std::endl;
        response->error_code = NET_ERR__INTERNAL_SERVER_ERROR;
        return;
    }

    response->error_code = NET_ERR__NO_ERROR;
}

}

