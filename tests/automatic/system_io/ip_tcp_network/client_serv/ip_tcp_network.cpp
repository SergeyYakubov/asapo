#include <iostream>
#include "io/io_factory.h"
#include <chrono>
#include <thread>
#include <future>
#include <stdio.h>

#include "testing.h"

using asapo::Error;
using asapo::ErrorType;
using asapo::AddressFamilies;
using asapo::SocketTypes;
using asapo::SocketProtocols;
using asapo::FileDescriptor;

using namespace std::chrono;

static const std::unique_ptr<asapo::IO> io(asapo::GenerateDefaultIO());
static const std::string kListenAddress = "127.0.0.1:60123";
static std::promise<void> kThreadStarted;
static const int kNumberOfChecks = 2;
static const size_t kSkipAmount = 500;

void Exit(int exit_number) {
    std::cerr << "ERROR: Exit on " << exit_number << std::endl;
    exit(exit_number);
}

void ExitIfErrIsNotOk(Error* err, int exit_number) {
    if(*err != nullptr) {
        std::cerr << "Explain(): " << (*err)->Explain() << std::endl;
        Exit(exit_number);
    }
}

std::unique_ptr<std::thread> CreateEchoServerThread() {
    return io->NewThread("EchoServer", [&] {
        Error err;
        FileDescriptor socket = io->CreateSocket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP, &err);
        ExitIfErrIsNotOk(&err, 100);
        io->InetBind(socket, kListenAddress, &err);
        std::cout << "[SERVER] Listen" << std::endl;
        ExitIfErrIsNotOk(&err, 101);
        io->Listen(socket, 5, &err);
        ExitIfErrIsNotOk(&err, 102);
        kThreadStarted.set_value();

        for(int i = 0; i < kNumberOfChecks; i++) {
            std::cout << "[SERVER][" << i << "] InetAcceptConnection" << std::endl;
            auto client_info_tuple = io->InetAcceptConnection(socket, &err);
            ExitIfErrIsNotOk(&err, 103);
            std::string client_address;
            FileDescriptor client_fd;
            std::tie(client_address, client_fd) = *client_info_tuple;

            ExitIfErrIsNotOk(&err, 104);
            while (true) {
                uint64_t need_to_receive_size;
                io->ReceiveWithTimeout(client_fd, &need_to_receive_size, sizeof(uint64_t), 100, &err);
                if(err != nullptr) {
                    if (asapo::IOErrorTemplates::kTimeout == err) {
                        continue;
                    }
                    if (asapo::ErrorTemplates::kEndOfFile == err) {
                        break;
                    }
                }
                ExitIfErrIsNotOk(&err, 105);//ReceiveWithTimeout

                std::unique_ptr<uint8_t[]> buffer(new uint8_t[need_to_receive_size]);

                io->Skip(client_fd, kSkipAmount, &err);
                ExitIfErrIsNotOk(&err, 106);

                size_t received = io->Receive(client_fd, buffer.get(), need_to_receive_size, &err);
                io->Send(client_fd, buffer.get(), received, &err);
                ExitIfErrIsNotOk(&err, 107);

                io->ReceiveDataToFile(client_fd, "", "received", need_to_receive_size, false, true);
                buffer = io->GetDataFromFile("received", &need_to_receive_size, &err);
                io->Send(client_fd, buffer.get(), received, &err);
                ExitIfErrIsNotOk(&err, 108);
            }

            std::cout << "[SERVER][" << i << "] Close client_fd" << std::endl;
            io->CloseSocket(client_fd, &err);
            ExitIfErrIsNotOk(&err, 109);
        }
        std::cout << "[SERVER] Close server socket" << std::endl;
        io->CloseSocket(socket, &err);
        ExitIfErrIsNotOk(&err, 110);
    });
}

void CheckNormal(int times, size_t size) {
    Error err;
    std::cout << "[CLIENT] CreateAndConnectIPTCPSocket" << std::endl;
    FileDescriptor socket = io->CreateAndConnectIPTCPSocket(kListenAddress, &err);
    ExitIfErrIsNotOk(&err, 201);

    std::cout << "[CLIENT] ReceiveWithTimeout" << std::endl;
    io->ReceiveWithTimeout(socket, nullptr, 1, 1000 * 100/*100ms*/, &err);
    if (asapo::IOErrorTemplates::kTimeout != err) {
        ExitIfErrIsNotOk(&err, 202);
    }

    for (int i = 0; i < times; i++) {
        std::cout << "[CLIENT] Allocate and create random numbers" << std::endl;
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[size]);
        for (size_t i = 0; i < size; i++) {
            buffer[i] = rand();
        }

        FILE* out = fopen("sent", "wb");
        fwrite(buffer.get(), 1, size, out);
        fclose(out);

        uint64_t send_size = size;

        std::cout << "[CLIENT] Send Size" << std::endl;
        io->Send(socket, &send_size, sizeof(uint64_t), &err);
        ExitIfErrIsNotOk(&err, 203);

        std::cout << "[CLIENT] Send data to skip" << std::endl;
        io->Send(socket, buffer.get(), kSkipAmount, &err);
        ExitIfErrIsNotOk(&err, 204);

        std::cout << "[CLIENT] Send" << std::endl;
        io->Send(socket, buffer.get(), size, &err);
        ExitIfErrIsNotOk(&err, 205);

        std::unique_ptr<uint8_t[]> buffer2(new uint8_t[size]);
        std::cout << "[CLIENT] Receive" << std::endl;
        size_t receive_count = io->Receive(socket, buffer2.get(), size, &err);
        ExitIfErrIsNotOk(&err, 206);
        if(receive_count != size) {
            Exit(205);
        }

        std::cout << "[CLIENT] buffer check" << std::endl;
        for (size_t i = 0; i < size; i++) {
            if (buffer[i] != buffer2[i]) {
                Exit(207);
            }
        }

        std::cout << "[CLIENT] send file" << std::endl;
        err = io->SendFile(socket, "sent", size);
        ExitIfErrIsNotOk(&err, 208);
        std::cout << "[CLIENT] receive file" << std::endl;
        receive_count = io->Receive(socket, buffer2.get(), size, &err);
        ExitIfErrIsNotOk(&err, 209);
        if(receive_count != size) {
            Exit(210);
        }
        std::cout << "[CLIENT] buffer check after receive file" << std::endl;
        for (size_t i = 0; i < size; i++) {
            if (buffer[i] != buffer2[i]) {
                Exit(211);
            }
        }


    }

    std::cout << "[CLIENT] Close" << std::endl;
    io->CloseSocket(socket, &err);
    ExitIfErrIsNotOk(&err, 108);
}

int main(int argc, char* argv[]) {
    Error err;
    std::cout << "[META] Check if connection is refused if server is not running" << std::endl;
    io->CreateAndConnectIPTCPSocket(kListenAddress, &err);
    if(asapo::IOErrorTemplates::kConnectionRefused != err) {
        ExitIfErrIsNotOk(&err, 301);
    }

    std::cout << "[META] Check invalid address format - Missing port" << std::endl;
    io->CreateAndConnectIPTCPSocket("localhost", &err);
    if(asapo::IOErrorTemplates::kInvalidAddressFormat != err) {
        ExitIfErrIsNotOk(&err, 302);
    }

    std::cout << "[META] Check unknown host" << std::endl;
    io->CreateAndConnectIPTCPSocket("some-host-that-might-not-exists.aa:1234", &err);
    if(err != asapo::IOErrorTemplates::kUnableToResolveHostname) {
        ExitIfErrIsNotOk(&err, 303);
    }

    std::unique_ptr<std::thread> server_thread = CreateEchoServerThread();
    kThreadStarted.get_future().get();//Make sure that the server is started

    std::cout << "Check 1" << std::endl;
    CheckNormal(10, 1024 * 1024 * 3);//3 MiByte
    std::cout << "Check 2" << std::endl;
    CheckNormal(30, 1024);//1 KiByte

    std::cout << "server_thread->join()" << std::endl;
    server_thread->join();

    std::cout << "[META] Check if connection is refused after server is closed" << std::endl;
    io->CreateAndConnectIPTCPSocket(kListenAddress, &err);
    if(asapo::IOErrorTemplates::kConnectionRefused != err) {
        ExitIfErrIsNotOk(&err, 304);
    }
    remove("sent");
    remove("received");

    return 0;
}
