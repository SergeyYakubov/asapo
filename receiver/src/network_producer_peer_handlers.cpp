#include "network_producer_peer.h"

namespace hidra2 {

void hexDump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

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
        (NetworkProducerPeer::RequestHandler) &NetworkProducerPeer::handle_prepare_send_data_request_
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
    std::cout << "op_code " << request->op_code << std::endl;
    std::cout << "request_id " << request->request_id << std::endl;

    std::cout << "filename " << request->filename << std::endl;
    std::cout << "file_size " << request->file_size << std::endl;

    response->error_code = NET_ERR__NO_ERROR;
    response->file_reference_id = 2;
}

void NetworkProducerPeer::handle_send_data_chunk_request_(NetworkProducerPeer* self,
                                                          const SendDataChunkRequest* request,
                                                          SendDataChunkResponse* response) {
    std::cout << "op_code " << request->op_code << std::endl;
    std::cout << "request_id " << request->request_id << std::endl;

    std::cout << "file_reference_id " << request->file_reference_id << std::endl;
    std::cout << "start_byte " << request->start_byte << std::endl;
    std::cout << "chunk_size " << request->chunk_size << std::endl;

    void* chunk_buffer = malloc(request->chunk_size);

    self->io->recv(self->socket_fd_, chunk_buffer, request->chunk_size, 0);

    hexDump("response", chunk_buffer, request->chunk_size);

    free(chunk_buffer);
    response->error_code = NET_ERR__NO_ERROR;
}

}

