#include <iostream>
#include <future>
#include <common/error.h>
#include <logger/logger.h>
#include <testing.h>
#include <asapo_fabric/asapo_fabric.h>
#include <common/io_error.h>

using namespace asapo;
using namespace fabric;

std::promise<void> serverShutdown;
std::future<void> serverShutdown_future = serverShutdown.get_future();

std::promise<void> serverShutdownAck;
std::future<void> serverShutdownAck_future = serverShutdownAck.get_future();

void ServerMasterThread(const std::string& hostname, uint16_t port) {
    {
        Error err;
        auto log = CreateDefaultLoggerBin("AutomaticTesting");

        auto factory = GenerateDefaultFabricFactory();

        auto server = factory->CreateAndBindServer(log.get(), hostname, port, &err);
        M_AssertEq(nullptr, err, "factory->CreateAndBindServer");

        // Wait for client to send a request and then shutdown the server
        int dummyBuffer;
        FabricAddress clientAddress;
        FabricMessageId messageId;

        // In order to run the tests more stable. Otherwise a timeout could occurred with valgrind
        int tries = 0;
        do {
            err = nullptr;
            server->RecvAny(&clientAddress, &messageId, &dummyBuffer, sizeof(dummyBuffer), &err);
        } while (err == IOErrorTemplates::kTimeout && tries++ < 2);
        M_AssertEq(nullptr, err, "server->RecvAny");

        server->Send(clientAddress, messageId, &dummyBuffer, sizeof(dummyBuffer), &err);
        M_AssertEq(nullptr, err, "server->Send");

        serverShutdown_future.wait();
    }

    printf("Server is now down!\n");
    serverShutdownAck.set_value();
}

void ClientThread(const std::string& hostname, uint16_t port) {
    Error err;

    auto factory = GenerateDefaultFabricFactory();

    auto client = factory->CreateClient(&err);
    M_AssertEq(nullptr, err, "factory->CreateClient");

    auto serverAddress = client->AddServerAddress(hostname + ":" + std::to_string(port), &err);
    M_AssertEq(nullptr, err, "client->AddServerAddress");

    int dummyBuffer = 0;
    client->Send(serverAddress, 0, &dummyBuffer, sizeof(dummyBuffer), &err);
    M_AssertEq(nullptr, err, "client->Send");

    client->Recv(serverAddress, 0, &dummyBuffer, sizeof(dummyBuffer), &err);
    M_AssertEq(nullptr, err, "client->Recv");

    // Server should not respond to this message
    std::cout <<
              "The following call might take a while since its able to reach the server but the server is not responding"
              << std::endl;
    client->Recv(serverAddress, 0, &dummyBuffer, sizeof(dummyBuffer), &err);
    M_AssertEq(IOErrorTemplates::kTimeout, err, "client->Recv");
    err = nullptr;

    serverShutdown.set_value();
    serverShutdownAck_future.wait();

    // Server is now down
    client->Recv(serverAddress, 1, &dummyBuffer, sizeof(dummyBuffer), &err);
    M_AssertEq(FabricErrorTemplates::kInternalConnectionError, err, "client->Recv");
    err = nullptr;

    client->Send(serverAddress, 2, &dummyBuffer, sizeof(dummyBuffer), &err);
    M_AssertEq(FabricErrorTemplates::kInternalConnectionError, err, "client->Send");
    err = nullptr;
}

int main(int argc, char* argv[]) {
    std::string hostname = "127.0.0.1";
    uint16_t port = 1816;

    if (argc > 3) {
        std::cout << "Usage: " << argv[0] << " [<host>] [<port>]" << std::endl;
        return 1;
    }
    if (argc == 2) {
        hostname = argv[1];
    }
    if (argc == 3) {
        port = (uint16_t) strtoul(argv[2], nullptr, 10);
    }

    std::thread serverThread(ServerMasterThread, hostname, port);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    ClientThread(hostname, port);

    std::cout << "Done testing. Joining server" << std::endl;
    serverThread.join();

    return 0;
}
