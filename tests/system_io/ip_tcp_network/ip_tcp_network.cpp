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
static std::promise<void> kThreadStarted;
static const int kNumberOfChecks = 3;

void Exit(int exit_number) {
    std::cerr << "ERROR: Exit on " << exit_number << std::endl;
    getchar();
    exit(exit_number);
}

void ExitIfErrIsNotOk(IOErrors* err, int exit_number) {
    if(*err != IOErrors::kNoError) {
        Exit(exit_number);
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
        kThreadStarted.set_value();

        for(int i = 0; i < kNumberOfChecks; i++) {
            std::cout << "[SERVER][" << i << "] InetAccept" << std::endl;
            auto client_info_tuple = io->InetAccept(socket, &err);
            ExitIfErrIsNotOk(&err, 103);
            std::string client_address;
            FileDescriptor client_fd;
            std::tie(client_address, client_fd) = *client_info_tuple;

            ExitIfErrIsNotOk(&err, 104);
            while (true) {
                uint64_t need_to_receive_size;
                io->ReceiveTimeout(client_fd, &need_to_receive_size, sizeof(uint64_t), 100, &err);
                if (err == IOErrors::kTimeout) {
                    continue;
                }
                if (err == IOErrors::kEndOfFile) {
                    break;
                }
                std::unique_ptr<uint8_t[]> buffer(new uint8_t[need_to_receive_size]);
                size_t received = io->Receive(client_fd, buffer.get(), need_to_receive_size, &err);
                io->Send(client_fd, buffer.get(), received, &err);
                ExitIfErrIsNotOk(&err, 106);
            }

            std::cout << "[SERVER][" << i << "] Close client_fd" << std::endl;
            io->CloseSocket(client_fd, &err);
            ExitIfErrIsNotOk(&err, 107);
        }
        std::cout << "[SERVER] Close server socket" << std::endl;
        io->CloseSocket(socket, &err);
        ExitIfErrIsNotOk(&err, 108);
    });
}

void CheckNormal(int times, size_t size) {
    IOErrors err;
    std::cout << "[CLIENT] CreateAndConnectIPTCPSocket" << std::endl;
    FileDescriptor socket = io->CreateAndConnectIPTCPSocket(kListenAddress, &err);
    ExitIfErrIsNotOk(&err, 201);

    std::cout << "[CLIENT] ReceiveTimeout" << std::endl;
    io->ReceiveTimeout(socket, nullptr, 1, 1000 * 100/*100ms*/, &err);
    if (err != IOErrors::kTimeout) {
        ExitIfErrIsNotOk(&err, 202);
    }

    for (int i = 0; i < times; i++) {
        std::cout << "[CLIENT] Allocate and create random numbers" << std::endl;
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[size]);
        for (size_t i = 0; i < size; i++) {
            buffer[i] = rand();
        }

        uint64_t send_size = size;

        std::cout << "[CLIENT] Send Size" << std::endl;
        io->Send(socket, &send_size, sizeof(uint64_t), &err);
        ExitIfErrIsNotOk(&err, 203);

        std::cout << "[CLIENT] Send" << std::endl;
        io->Send(socket, buffer.get(), size, &err);
        ExitIfErrIsNotOk(&err, 203);

        std::unique_ptr<uint8_t[]> buffer2(new uint8_t[size]);
        std::cout << "[CLIENT] Receive" << std::endl;
        size_t receive_count = io->Receive(socket, buffer2.get(), size, &err);
        ExitIfErrIsNotOk(&err, 204);
        if(receive_count != size) {
            Exit(205);
        }

        std::cout << "[CLIENT] buffer check" << std::endl;
        for (size_t i = 0; i < size; i++) {
            if (buffer[i] != buffer2[i]) {
                Exit(206);
            }
        }
    }

    std::cout << "[CLIENT] Close" << std::endl;
    io->CloseSocket(socket, &err);
    ExitIfErrIsNotOk(&err, 107);
}

int main(int argc, char* argv[]) {
    std::thread* server_thread = CreateEchoServerThread();
    kThreadStarted.get_future().get();//Make sure that the server is started

    std::cout << "Check 1" << std::endl;
    CheckNormal(10, 1024 * 1024 * 3);
    std::cout << "Check 2" << std::endl;
    CheckNormal(30, 1024);
    std::cout << "Check 3" << std::endl;
    CheckNormal(2, 1024 * 1024 * 256/*256 MiByte */);

    std::cout << "server_thread->join()" << std::endl;
    server_thread->join();

    return 0;
}
