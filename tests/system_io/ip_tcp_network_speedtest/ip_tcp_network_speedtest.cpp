#include <iostream>
#include <system_wrappers/system_io.h>
#include <future>
#include <iomanip>

#include "testing.h"

using hidra2::Error;
using hidra2::ErrorType;
using hidra2::SystemIO;
using hidra2::AddressFamilies;
using hidra2::SocketTypes;
using hidra2::SocketProtocols;
using hidra2::FileDescriptor;
using hidra2::M_AssertEq;

using namespace std::chrono;
using std::chrono::high_resolution_clock;

static const std::unique_ptr<SystemIO> io(new SystemIO());
static const std::string kListenAddress = "127.0.0.1:4208";
static std::promise<void> kThreadStarted;

static size_t kTestSize = size_t(1024) * size_t(1024) * size_t(512); //512MiByte
static int kTestCount = 20;

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
    return io->NewThread([] {
        std::unique_ptr<uint8_t[]> kBufferServer(new uint8_t[kTestSize]);

        Error err;
        FileDescriptor socket = io->CreateSocket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP, &err);
        ExitIfErrIsNotOk(&err, 100);
        io->InetBind(socket, kListenAddress, &err);
        std::cout << "[SERVER] Bind on: " << kListenAddress << std::endl;
        std::cout << "[SERVER] Listen" << std::endl;
        ExitIfErrIsNotOk(&err, 101);
        io->Listen(socket, 5, &err);
        ExitIfErrIsNotOk(&err, 102);
        kThreadStarted.set_value();

        std::cout << "[SERVER] InetAccept" << std::endl;
        auto client_info_tuple = io->InetAccept(socket, &err);
        ExitIfErrIsNotOk(&err, 103);
        std::string client_address;
        FileDescriptor client_fd;
        std::tie(client_address, client_fd) = *client_info_tuple;

        for(int i = 0; i < kTestCount; i++) {
            io->Receive(client_fd, kBufferServer.get(), kTestSize, &err);
            ExitIfErrIsNotOk(&err, 105);
        }

        std::cout << "[SERVER] Close client_fd" << std::endl;
        io->CloseSocket(client_fd, &err);
        ExitIfErrIsNotOk(&err, 106);

        std::cout << "[SERVER] Close server socket" << std::endl;
        io->CloseSocket(socket, &err);
        ExitIfErrIsNotOk(&err, 107);
    });
}

void Speedtest() {
    std::unique_ptr<uint8_t[]> kBufferClient(new uint8_t[kTestSize]);

    Error err;
    std::cout << "[CLIENT] CreateAndConnectIPTCPSocket" << std::endl;
    FileDescriptor socket = io->CreateAndConnectIPTCPSocket(kListenAddress, &err);
    ExitIfErrIsNotOk(&err, 201);

    for(int i = 0; i < kTestCount; i++) {
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        io->Send(socket, kBufferClient.get(), kTestSize, &err);
        ExitIfErrIsNotOk(&err, 203);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();

        double tookMs = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
        std::cout << i << ":\t" << std::setprecision(2) << std::fixed << (kTestSize / (tookMs / 1000)) / 1024 / 1024 / 1024 <<
                  " GiByte/s" <<
                  std::endl;
        /*
        << "\t"<< std::fixed << kTestSize/(tookMs/1000) << " Byte/s" << std::endl
        << "\tTime " << tookMs << "ms " << std::endl
        << "\tfor  " << kTestSize << " Byte" << std::endl;
        */
    }

    std::cout << "[CLIENT] Close" << std::endl;
    io->CloseSocket(socket, &err);
    ExitIfErrIsNotOk(&err, 204);
}

int main(int argc, char* argv[]) {
    std::unique_ptr<std::thread> server_thread = CreateEchoServerThread();
    kThreadStarted.get_future().get();//Make sure that the server is started

    Speedtest();

    std::cout << "server_thread->join()" << std::endl;
    server_thread->join();

    return 0;
}
