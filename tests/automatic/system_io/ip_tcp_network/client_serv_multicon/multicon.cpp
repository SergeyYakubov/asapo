#include <iostream>
#include "io/io_factory.h"
#include <chrono>
#include <thread>
#include <future>
#include <atomic>
#include <algorithm>

#include "testing.h"

using asapo::Error;
using asapo::ErrorType;
using asapo::AddressFamilies;
using asapo::SocketTypes;
using asapo::SocketProtocols;
using asapo::SocketDescriptor;
using asapo::M_AssertEq;

using namespace std::chrono;

static const std::unique_ptr<asapo::IO> io(asapo::GenerateDefaultIO());
static const std::string kListenAddress = "127.0.0.1:60123";
static std::promise<void> kThreadStarted;
std::atomic<int> exit_thread{0};

void Exit(int exit_number) {
    std::cerr << "ERROR: Exit on " << exit_number << std::endl;
    exit(exit_number);
}

void ExitIfErrIsNotOk(Error* err, int exit_number) {
    if(*err != nullptr) {
        std::cerr << "Explain(): " << (*err)->Explain() << " exit number: " << exit_number << std::endl;
        Exit(exit_number);
    }
}

std::unique_ptr<std::thread> CreateEchoServerThread() {
    return io->NewThread("EchoServer", [&] {
        Error err;
        SocketDescriptor master_socket = io->CreateAndBindIPTCPSocketListener(kListenAddress, 3, &err);
        std::cout << "[SERVER] master socket " << master_socket << std::endl;
        ExitIfErrIsNotOk(&err, 100);
        kThreadStarted.set_value();
        asapo::ListSocketDescriptors sockets_to_listen;
        while (!exit_thread) {
            std::vector<std::string> new_connections;
            auto sockets = io->WaitSocketsActivity(master_socket, &sockets_to_listen, &new_connections, &err);
            if (err != asapo::IOErrorTemplates::kTimeout) {
                ExitIfErrIsNotOk(&err, 102);
            }
            for(auto socket : sockets) {
                std::cout << "[SERVER] processing socket " << socket << std::endl;
                uint64_t message;
                io->Receive(socket, &message, sizeof(uint64_t), &err);
                if (err == asapo::ErrorTemplates::kEndOfFile) {
                    std::cout << "[SERVER] end of file " << socket << std::endl;
                    io->CloseSocket(socket, &err);
                    ExitIfErrIsNotOk(&err, 106);
                    std::cout << "[SERVER] socket closed " << socket << std::endl;
                    sockets_to_listen.erase(std::remove(sockets_to_listen.begin(), sockets_to_listen.end(), socket),
                                            sockets_to_listen.end());
                    continue;
                }
                ExitIfErrIsNotOk(&err, 104);
                io->Send(socket, &message, sizeof(uint64_t), &err);
                ExitIfErrIsNotOk(&err, 105);
            }
        }
        for(auto socket : sockets_to_listen) {
            std::cout << "[SERVER] close socket " << socket << std::endl;
            io->CloseSocket(socket, &err);
            ExitIfErrIsNotOk(&err, 108);
        }
        io->CloseSocket(master_socket, &err);
        std::cout << "[SERVER] finished" << std::endl;
    });
}

void CheckNormal(int times) {
    Error err;
    std::cout << "[CLIENT] CreateAndConnectIPTCPSocket" << std::endl;
    SocketDescriptor socket = io->CreateAndConnectIPTCPSocket(kListenAddress, &err);
    ExitIfErrIsNotOk(&err, 201);


    for (int i = 0; i < times; i++) {
        std::cout << "[CLIENT] send random number" << std::endl;

        uint64_t message_send = rand();

        std::cout << "[CLIENT] Send Size" << std::endl;
        io->Send(socket, &message_send, sizeof(uint64_t), &err);
        ExitIfErrIsNotOk(&err, 203);

        uint64_t message_recv;
        io->Receive(socket, &message_recv, sizeof(uint64_t), &err);
        ExitIfErrIsNotOk(&err, 206);
        if(message_recv != message_send) {
            Exit(205);
        }
    }
    std::cout << "[CLIENT] Close" << std::endl;
    io->CloseSocket(socket, &err);
    std::cout << "[CLIENT] socket closed" << std::endl;
    ExitIfErrIsNotOk(&err, 108);
}

int main(int argc, char* argv[]) {
    Error err;
    std::unique_ptr<std::thread> server_thread = CreateEchoServerThread();
    //server_thread->detach();
    kThreadStarted.get_future().get();//Make sure that the server is started

    std::cout << "Check" << std::endl;
    auto thread1 = io->NewThread("CheckNormal 1",  [&] {
        CheckNormal(30);
    });
    auto thread2 = io->NewThread("CheckNormal 2", [&] {
        CheckNormal(30);
    });
    auto thread3 = io->NewThread("CheckNormal 3", [&] {
        CheckNormal(30);
    });

    thread1->join();
    thread2->join();
    thread3->join();

    exit_thread = 1;

    std::cout << "server_thread->join()" << std::endl;
    server_thread->join();

    return 0;
}
