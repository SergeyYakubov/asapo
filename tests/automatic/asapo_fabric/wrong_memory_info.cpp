#include <asapo/common/error.h>
#include <asapo/asapo_fabric/asapo_fabric.h>
#include <testing.h>
#include <thread>
#include <iostream>
#include <cstring>
#include <future>
#include "asapo/request/request.h"

using namespace asapo;
using namespace fabric;

std::promise<void> clientIsDone;
std::future<void> clientIsDoneFuture = clientIsDone.get_future();

std::promise<void> serverIsDone;
std::future<void> serverIsDoneFuture = serverIsDone.get_future();

constexpr size_t kRdmaSize = 5 * 1024;
constexpr size_t kDummyDataSize = 512;

void ServerMasterThread(const std::string& hostname, uint16_t port) {
    Error err;
    auto log = CreateDefaultLoggerBin("AutomaticTesting");

    auto factory = GenerateDefaultFabricFactory();
    auto server = factory->CreateAndBindServer(log.get(), hostname, port, &err);
    M_AssertEq(nullptr, err, "factory->CreateAndBindServer");

    GenericRequestHeader request{};

    auto rdmaBuffer = std::unique_ptr<char[]>(new char[kRdmaSize]);
    auto dummyData = std::unique_ptr<char[]>(new char[kDummyDataSize]);

    FabricAddress clientAddress;
    FabricMessageId messageId;

    // Simulate faulty memory details
    server->RecvAny(&clientAddress, &messageId, &request, sizeof(request), &err);
    M_AssertEq(nullptr, err, "server->RecvAny(1)");
    M_AssertEq(1, messageId);
    M_AssertEq("Hello World", request.message);
    server->RdmaWrite(clientAddress, (MemoryRegionDetails*)&request.stream, rdmaBuffer.get(), kRdmaSize, &err);
    M_AssertEq(FabricErrorTemplates::kInternalError, err, "server->RdmaWrite(1)");
    err = nullptr; // We have to reset the error by ourselves
    server->Send(clientAddress, messageId, dummyData.get(), kDummyDataSize, &err);
    M_AssertEq(nullptr, err, "server->Send(1)");

    // Simulate correct memory details
    int tries = 0;
    do {
        err = nullptr;
        server->RecvAny(&clientAddress, &messageId, &request, sizeof(request), &err);
    } while (err == IOErrorTemplates::kTimeout && tries++ < 2);
    M_AssertEq(nullptr, err, "server->RecvAny(2)");
    M_AssertEq(2, messageId);
    server->RdmaWrite(clientAddress, (MemoryRegionDetails*)&request.stream, rdmaBuffer.get(), kRdmaSize, &err);
    M_AssertEq(nullptr, err, "server->RdmaWrite(2)");
    server->Send(clientAddress, messageId, dummyData.get(), kDummyDataSize, &err);
    M_AssertEq(nullptr, err, "server->Send(2)");

    // Simulate old (unregistered) memory details
    GenericRequestHeader request2{};
    tries = 0;
    do {
        err = nullptr;
        server->RecvAny(&clientAddress, &messageId, &request2, sizeof(request2), &err);
    } while (err == IOErrorTemplates::kTimeout && tries++ < 2);
    M_AssertEq(nullptr, err, "server->RecvAny(3)");
    M_AssertEq(3, messageId);
    server->RdmaWrite(clientAddress, (MemoryRegionDetails*)&request.stream, rdmaBuffer.get(), kRdmaSize, &err);
    M_AssertEq(FabricErrorTemplates::kInternalError, err, "server->RdmaWrite(3)");

    std::cout << "[SERVER] Waiting for client to finish" << std::endl;
    clientIsDoneFuture.get();
    serverIsDone.set_value();
}

void ClientThread(const std::string& hostname, uint16_t port) {
    Error err;

    auto factory = GenerateDefaultFabricFactory();

    auto client = factory->CreateClient(&err);
    M_AssertEq(nullptr, err, "factory->CreateClient");

    auto serverAddress = client->AddServerAddress(hostname + ":" + std::to_string(port), &err);
    M_AssertEq(nullptr, err, "client->AddServerAddress");

    auto actualRdmaBuffer = std::unique_ptr<char[]>(new char[kRdmaSize]);
    auto dummyData = std::unique_ptr<char[]>(new char[kDummyDataSize]);

    GenericRequestHeader request{};
    FabricMessageId messageId = 1;
    strcpy(request.message, "Hello World");

    // Scoped MemoryRegion
    {
        auto mr = client->ShareMemoryRegion(actualRdmaBuffer.get(), kRdmaSize, &err);
        M_AssertEq(nullptr, err, "client->ShareMemoryRegion");
        memcpy(request.stream, mr->GetDetails(), sizeof(MemoryRegionDetails));

        // Simulate faulty memory details
        ((MemoryRegionDetails*)(&request.stream))->key++;
        client->Send(serverAddress, messageId, &request, sizeof(request), &err);
        M_AssertEq(nullptr, err, "client->Send(1)");
        client->Recv(serverAddress, messageId, dummyData.get(), kDummyDataSize, &err);
        M_AssertEq(nullptr, err, "client->Recv(1)");
        messageId++;

        // Simulate correct memory details
        memcpy(request.stream, mr->GetDetails(), sizeof(MemoryRegionDetails));
        client->Send(serverAddress, messageId, &request, sizeof(request), &err);
        M_AssertEq(nullptr, err, "client->Send(2)");
        client->Recv(serverAddress, messageId, dummyData.get(), kDummyDataSize, &err);
        M_AssertEq(nullptr, err, "client->Recv(2)");
        messageId++;
    }

    // Simulate old (unregistered) memory details
    // Details are still written from "Simulate correct memory details"
    client->Send(serverAddress, messageId, &request, sizeof(request), &err);
    M_AssertEq(nullptr, err, "client->Send(3)");

    clientIsDone.set_value();
    std::cout << "[Client] Waiting for server to finish" << std::endl;
    serverIsDoneFuture.get();
}

int main(int argc, char* argv[]) {
    std::string hostname = "127.0.0.1";
    uint16_t port = 1816;

    if (argc > 3) {
        std::cout << "Usage: " << argv[0] << " [<host>] [<port>]" << std::endl;
        return EXIT_FAILURE;
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

    return EXIT_SUCCESS;
}
