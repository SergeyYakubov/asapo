#include "network_producer_peer.h"
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cmath>
#include <zconf.h>

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

    if(self->got_hello_) {
        std::cerr << "Client deprecated_send hello twice." << std::endl;
        self->io->deprecated_close(self->socket_fd_);
        return;
    }
    self->got_hello_ = true;

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
    FileReferenceHandlerError error = FILE_REFERENCE_HANDLER_ERR__OK;
    FileReferenceId reference_id = self->file_reference_handler.add_file(request->filename,
                                                                         request->file_size,
                                                                         self->connection_id(),
                                                                         error);

    if(reference_id == 0 || error) {
        std::cerr << "Failed to add_file. FileReferenceHandlerError: " << error << std::endl;
        response->error_code = NET_ERR__ALLOCATE_STORAGE_FAILED;
        response->file_reference_id = 0;
        return;
    }

    std::cout << "Created new file '" << request->filename << "' of size " << request->file_size << std::endl;

    response->error_code = NET_ERR__NO_ERROR;
    response->file_reference_id = reference_id;
}

void NetworkProducerPeer::handle_send_data_chunk_request_(NetworkProducerPeer* self,
                                                          const SendDataChunkRequest* request,
                                                          SendDataChunkResponse* response) {
    IOErrors io_error;
    /*
    std::cout << "[CHUNK]op_code " << request->op_code << std::endl;
    std::cout << "[CHUNK]request_id " << request->request_id << std::endl;

    std::cout << "[CHUNK]file_reference_id " << request->file_reference_id << std::endl;
    std::cout << "[CHUNK]start_byte " << request->start_byte << std::endl;
    std::cout << "[CHUNK]chunk_size " << request->chunk_size << std::endl;
    */
    auto file_info = self->file_reference_handler.get_file(request->file_reference_id);

    if(file_info == nullptr || file_info->owner != self->connection_id()) {
        response->error_code = NET_ERR__UNKNOWN_REFERENCE_ID;
        return;
    }

    // Round to the next full pagesize
    size_t map_start = size_t(request->start_byte/getpagesize())*getpagesize();
    size_t map_offset = request->start_byte%getpagesize();
    size_t map_size = static_cast<size_t>(ceil(float(request->chunk_size+map_offset)/float(getpagesize()))*getpagesize());

    if(request->start_byte+request->chunk_size > file_info->file_size) {
        std::cerr << "Producer is sending a lager file then excepted" << std::endl;
        self->io->receive_timeout(self->socket_fd_, nullptr, request->chunk_size, 30, &io_error);//TODO nullptr not a valid target for receive
        return;
    }

    void* mapped_file = mmap(nullptr,
                             map_size,
                             PROT_READ | PROT_WRITE, MAP_SHARED,
                             file_info->fd,
                             map_start);

    if(!mapped_file || mapped_file == MAP_FAILED) {
        std::cerr << "Mapping a file failed. errno: " << errno << std::endl;
        self->io->receive_timeout(self->socket_fd_, nullptr, request->chunk_size, 30, &io_error);//TODO nullptr not a valid target for receive
        response->error_code = NET_ERR__INTERNAL_SERVER_ERROR;
        return;
    }

    self->io->receive_timeout(self->socket_fd_, mapped_file + map_offset, request->chunk_size, 30, &io_error);
    if(io_error != IOErrors::NO_ERROR) {
        std::cerr << "Fail to receive all the chunk data." << std::endl;
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

