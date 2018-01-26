#include <iostream>
#include <system_wrappers/system_io.h>
#include <chrono>
#include <thread>
#include <future>

#include "testing.h"

using hidra2::SystemIO;
using hidra2::IOErrors;
using hidra2::AddressFamilies;
using hidra2::SocketTypes;
using hidra2::SocketProtocols;
using hidra2::FileDescriptor;
using hidra2::M_AssertEq;

using namespace std::chrono;

static const std::unique_ptr<SystemIO> io(new SystemIO());
static const std::string kListenAddress = "127.0.0.1:60123";
static std::promise<void> thread_started;

void ExitIfErrIsNotOk(IOErrors* err, int exit_number) {
    if(*err != IOErrors::kNoError) {
        std::cerr << "ERROR: Exit on " << exit_number << std::endl;
        exit(exit_number);
    }
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
        thread_started.set_value();

        int i = 0;
        while (true) {
            std::cout << "[SERVER][" << i << "] InetAccept" << std::endl;
            auto client_info_tuple = io->InetAccept(socket, &err);
            ExitIfErrIsNotOk(&err, 103);
            std::string client_address;
            FileDescriptor client_fd;
            std::tie(client_address, client_fd) = *client_info_tuple;

            size_t max_buffer_size = 1024 * 1024;//1MiByte
            std::unique_ptr<uint8_t[]> buffer(new uint8_t[max_buffer_size]);
            while (true) {
                size_t received = io->ReceiveTimeout(client_fd, buffer.get(), max_buffer_size, 100, &err);
                if (err == IOErrors::kTimeout) {
                    continue;
                }
                if (err == IOErrors::kEndOfFile) {
                    io->Send(client_fd, buffer.get(), received, &err);
                    ExitIfErrIsNotOk(&err, 104);
                    break;
                }
                ExitIfErrIsNotOk(&err, 104);
                io->Send(client_fd, buffer.get(), received, &err);
                ExitIfErrIsNotOk(&err, 105);
            }

            std::cout << "[SERVER][" << i << "] Close client_fd" << std::endl;
            io->CloseSocket(client_fd, &err);
            ExitIfErrIsNotOk(&err, 106);
            break;
        }
        std::cout << "[SERVER][" << i << "] Close socket" << std::endl;
        io->CloseSocket(socket, &err);
        ExitIfErrIsNotOk(&err, 107);
    });
}

void CheckNormal(int times, size_t size) {
    IOErrors err;
    std::cout << "[CLIENT] CreateAndConnectIPTCPSocket" << std::endl;
    FileDescriptor socket = io->CreateAndConnectIPTCPSocket(kListenAddress, &err);
    ExitIfErrIsNotOk(&err, 201);

    io->ReceiveTimeout(socket, nullptr, 1, 1000 * 100/*100ms*/, &err);
    if (err != IOErrors::kTimeout) {
        ExitIfErrIsNotOk(&err, 202);
    }

    for (int i = 0; i < times; i++) {
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[size]);
        for (size_t i = 0; i < size; i++) {
            buffer[i] = rand();
        }

        std::cout << "[CLIENT] Send" << std::endl;
        io->Send(socket, buffer.get(), size, &err);
        ExitIfErrIsNotOk(&err, 203);

        std::unique_ptr<uint8_t[]> buffer2(new uint8_t[size]);
        std::cout << "[CLIENT] Receive" << std::endl;
        io->Receive(socket, buffer2.get(), size, &err);
        ExitIfErrIsNotOk(&err, 204);

        std::cout << "[CLIENT] buffer check" << std::endl;
        for (size_t i = 0; i < size; i++) {
            if (buffer[i] != buffer2[i]) {
                exit(205);
            }
        }
    }

    std::cout << "[CLIENT] Close" << std::endl;
    io->CloseSocket(socket, &err);
    ExitIfErrIsNotOk(&err, 106);
}

int main(int argc, char* argv[]) {
    std::thread* server_thread = CreateEchoServerThread();
    server_thread->detach();
    thread_started.get_future().get();//Make sure that the server is started

    std::cout << "Check 1" << std::endl;
    CheckNormal(10, 1024 * 1024 * 3);
    std::cout << "Check 2" << std::endl;
    CheckNormal(30, 1024 * 1024 * 30);
    std::cout << "Check 3" << std::endl;
    CheckNormal(2, 1024 * 1024 * 1/*100 MiByte */);

    return 0;
}
