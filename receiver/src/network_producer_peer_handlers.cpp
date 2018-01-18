#include "network_producer_peer.h"
#include "receiver.h"
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cmath>
#include <zconf.h>

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
    IOError ioErr;

    if(request->file_size > 2*1024*1024*1024/*2GiByte*/) {
        response->error_code = NET_ERR__ALLOCATE_STORAGE_FAILED;
        return;
    }

    FileDescriptor fd = self->CreateAndOpenFileByFileId(request->file_id, &ioErr);
    if(ioErr != IOError::NO_ERROR) {
        response->error_code = NET_ERR__FILENAME_ALREADY_IN_USE;
        std::cerr << "[" << self->connection_id() << "] file_id: " << request->file_id << " does already exists" << std::endl;
        self->io->Skip(self->socket_fd_, request->file_size, &ioErr);
        if(ioErr != IOError::NO_ERROR) {
            std::cout << "[NetworkProducerPeer] Out of sync force disconnect" << std::endl;
            self->io->Close(self->socket_fd_);
        }
        return;
    }

    std::unique_ptr<uint8_t[]> buffer;
    try {
        buffer.reset(new uint8_t[request->file_size]);
    }
    catch(std::exception& e) {
        std::cerr << "[" << self->connection_id() << "] Failed to allocate enough memory. file_id: " << request->file_id << " file_size:" << request->file_size << std::endl;
        std::cerr << e.what() << std::endl;
        response->error_code = NET_ERR__INTERNAL_SERVER_ERROR;
        return;
    }

    self->io->Receive(self->socket_fd_, buffer.get(), request->file_size, &ioErr);
    if(ioErr != IOError::NO_ERROR) {
        std::cerr << "[" << self->connection_id() << "] An IO error occurred while receiving the file" << std::endl;
        response->error_code = NET_ERR__INTERNAL_SERVER_ERROR;
    }

    //self->io->Write(fd, buffer.get(), request->file_size, &ioErr);

    response->error_code = NET_ERR__NO_ERROR;
}

}

