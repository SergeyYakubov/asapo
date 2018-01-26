#include <iostream>
#include <system_wrappers/system_io.h>
#include <chrono>
#include <thread>

#include "testing.h"

using hidra2::SystemIO;
using hidra2::IOErrors;
using hidra2::AddressFamilies;
using hidra2::SocketTypes;
using hidra2::SocketProtocols;
using hidra2::FileDescriptor;
using hidra2::M_AssertEq;

using namespace std::chrono_literals;

static const std::unique_ptr<SystemIO> io(new SystemIO());
static const std::string kListenAddress = "127.0.0.1:60123";
static const size_t kSendBufferSize = 1024 * 1024 * 5; //3 MiByte

void ExitIfErrIsNotOk(IOErrors* err, int exit_number) {
    if(*err != IOErrors::kNoError)
        exit(exit_number);
}

std::thread* CreateEchoServerThread() {
    return io->NewThread([&] {
        IOErrors err;
        FileDescriptor socket = io->CreateSocket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP, &err);
        ExitIfErrIsNotOk(&err, 100);
        io->InetBind(socket, kListenAddress, &err);
        std::cout << "[SERVER] Listen" << std::endl;
        ExitIfErrIsNotOk(&err, 101);
        io->Listen(socket, 5, &err);
        ExitIfErrIsNotOk(&err, 102);

        std::cout << "[SERVER] InetAccept" << std::endl;
        auto client_info_tuple = io->InetAccept(socket, &err);
        ExitIfErrIsNotOk(&err, 103);
        std::string client_address;
        FileDescriptor client_fd;
        std::tie(client_address, client_fd) = *client_info_tuple;


        std::unique_ptr<uint8_t[]> buffer(new uint8_t[kSendBufferSize]);
        std::cout << "[SERVER] Receive" << std::endl;
        io->Receive(client_fd, buffer.get(), kSendBufferSize, &err);
        ExitIfErrIsNotOk(&err, 104);
        io->Send(client_fd, buffer.get(), kSendBufferSize, &err);
        std::cout << "[SERVER] Send" << std::endl;
        ExitIfErrIsNotOk(&err, 105);

        std::cout << "[SERVER] Close client_fd" << std::endl;
        io->Close(client_fd, &err);
        ExitIfErrIsNotOk(&err, 106);

        std::cout << "[SERVER] Close socket" << std::endl;
        io->Close(socket, &err);
        ExitIfErrIsNotOk(&err, 107);
    });
}

int main(int argc, char* argv[]) {
    std::thread* server_thread = CreateEchoServerThread();
    server_thread->detach();

	std::this_thread::sleep_for(2s); // Just to make sure that the server thread starts running

    IOErrors err;
    std::cout << "[CLIENT] CreateAndConnectIPTCPSocket" << std::endl;
    FileDescriptor socket = io->CreateAndConnectIPTCPSocket(kListenAddress, &err);
    ExitIfErrIsNotOk(&err, 201);

    std::unique_ptr<uint8_t[]> buffer(new uint8_t[kSendBufferSize]);
    for(size_t i = 0; i < kSendBufferSize; i++) {
        buffer[i] = (uint8_t)i;
    }

    std::cout << "[CLIENT] Send" << std::endl;
    io->Send(socket, buffer.get(), kSendBufferSize, &err);
    ExitIfErrIsNotOk(&err, 202);

    std::unique_ptr<uint8_t[]> buffer2(new uint8_t[kSendBufferSize]);
    std::cout << "[CLIENT] Receive" << std::endl;
    io->Receive(socket, buffer2.get(), kSendBufferSize, &err);
    ExitIfErrIsNotOk(&err, 203);

    std::cout << "[CLIENT] Close" << std::endl;
    io->Close(socket, &err);
    ExitIfErrIsNotOk(&err, 104);

    std::cout << "[CLIENT] buffer check" << std::endl;
    for(size_t i = 0; i < kSendBufferSize; i++) {
        if(buffer[i] != buffer2[i]) {
            exit(205);
        }
    }

    return 0;
}
