#include <system_wrappers/system_io.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include "producer_impl.h"

const uint32_t hidra2::ProducerImpl::kVersion = 1;

hidra2::ProducerImpl::ProducerImpl() {
    __set_io(ProducerImpl::kDefaultIO);
}

uint64_t hidra2::ProducerImpl::get_version() const {
    return kVersion;
}

hidra2::ProducerStatus hidra2::ProducerImpl::get_status() const {
    return PRODUCER_STATUS__CONNECTED;
}

hidra2::ProducerError hidra2::ProducerImpl::connect_to_receiver(std::string receiver_address) {
    if(client_fd_ != -1) {
        return PRODUCER_ERROR__ALREADY_CONNECTED;
    }

    client_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    sockaddr_in socket_address {};
    socket_address.sin_addr.s_addr = inet_addr(receiver_address.c_str());
    socket_address.sin_port = htons(8099);
    socket_address.sin_family = AF_INET;

    io->connect(client_fd_, (struct sockaddr *)&socket_address, sizeof(socket_address));

    hidra2::HelloRequest helloRequest;
    helloRequest.op_code = OP_CODE__HELLO;
    helloRequest.request_id = request_id++;
    helloRequest.client_version = 3;
    helloRequest.os = (OSType)4;
    helloRequest.is_x64 = true;

    io->send(client_fd_, &helloRequest, sizeof(helloRequest), 0);

    HelloResponse helloResponse;

    io->recv(client_fd_, &helloResponse, sizeof(helloResponse), 0);

    std::cout << "op_code: " << helloResponse.op_code << std::endl;
    std::cout << "request_id: " << helloResponse.request_id << std::endl;
    std::cout << "error_code: " << helloResponse.error_code << std::endl;
    std::cout << "server_version: " << helloResponse.server_version << std::endl;


    if(helloResponse.error_code == NET_ERR__NO_ERROR) {
        return PRODUCER_ERROR__OK;
    }

    std::cerr << "Fail to connect to server. Server response error code: " << helloResponse.error_code << std::endl;

    return PRODUCER_ERROR__OK;
}

//TODO not our code. need to be removed. Copy&Pasted from stackoverflow
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

hidra2::ProducerError hidra2::ProducerImpl::send(std::string filename, void *data, uint64_t file_size) {
    hidra2::PrepareSendDataRequest prepareSendDataRequest;
    prepareSendDataRequest.op_code = OP_CODE__PREPARE_SEND_DATA;
    prepareSendDataRequest.request_id = request_id++;
    prepareSendDataRequest.file_size = file_size;
    filename.copy((char*)&prepareSendDataRequest.filename, sizeof(prepareSendDataRequest.filename));

    //TODO Loop

    std::cout << "Send file: " << filename << std::endl;

    io->send(client_fd_, &prepareSendDataRequest, sizeof(prepareSendDataRequest), 0);

    hidra2::PrepareSendDataResponse prepareSendDataResponse;

    io->recv(client_fd_, &prepareSendDataResponse, sizeof(prepareSendDataResponse), 0);

    std::cout << "op_code: " << prepareSendDataResponse.op_code << std::endl;
    std::cout << "request_id: " << prepareSendDataResponse.request_id << std::endl;
    std::cout << "error_code: " << prepareSendDataResponse.error_code << std::endl;
    std::cout << "file_reference_id: " << prepareSendDataResponse.file_reference_id << std::endl;


    hidra2::SendDataChunkRequest sendDataChunkRequest;
    sendDataChunkRequest.op_code = OP_CODE__SEND_DATA_CHUNK;
    sendDataChunkRequest.request_id = request_id++;
    sendDataChunkRequest.start_byte = 0;
    sendDataChunkRequest.chunk_size = file_size;
    sendDataChunkRequest.file_reference_id = prepareSendDataResponse.file_reference_id;

    io->send(client_fd_, &sendDataChunkRequest, sizeof(sendDataChunkRequest), 0);


    hexDump("send", data, file_size);

    io->send(client_fd_, data, file_size, 0);


    hidra2::SendDataChunkResponse sendDataChunkResponse;

    io->recv(client_fd_, &sendDataChunkResponse, sizeof(sendDataChunkResponse), 0);

    std::cout << "op_code: " << sendDataChunkResponse.op_code << std::endl;
    std::cout << "request_id: " << sendDataChunkResponse.request_id << std::endl;
    std::cout << "error_code: " << sendDataChunkResponse.error_code << std::endl;
}
