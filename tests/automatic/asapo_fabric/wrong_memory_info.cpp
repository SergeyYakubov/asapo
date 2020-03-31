#include <common/error.h>
#include <asapo_fabric/asapo_fabric.h>
#include <testing.h>
#include <thread>
#include <iostream>
#include <cstring>
#include <future>
#include <request/request.h>

using namespace asapo;
using namespace fabric;

std::promise<void> clientIsDone;
std::future<void> clientIsDoneFuture = clientIsDone.get_future();

std::promise<void> serverIsDone;
std::future<void> serverIsDoneFuture = serverIsDone.get_future();

constexpr size_t kRdmaSize = 5 * 1024;
constexpr size_t kDummyDataSize = 512;

void ServerMasterThread() {
    Error err;
    auto log = CreateDefaultLoggerBin("AutomaticTesting");

    auto factory = GenerateDefaultFabricFactory();
    auto server = factory->CreateAndBindServer(log.get(), "127.0.0.1", 1816, &err);
    M_AssertEq(nullptr, err, "factory->CreateAndBindServer");

    GenericRequestHeader request{};

    char* rdmaBuffer = new char[kRdmaSize];
    char* dummyData = new char[kDummyDataSize];

    FabricAddress clientAddress;
    FabricMessageId messageId;

    // Simulate faulty memory details
    server->RecvAny(&clientAddress, &messageId, &request, sizeof(request), &err);
    M_AssertEq(nullptr, err, "server->RecvAny(1)");
    M_AssertEq(1, messageId);
    M_AssertEq("Hello World", request.message);
    server->RdmaWrite(clientAddress, (MemoryRegionDetails*)&request.substream, rdmaBuffer, kRdmaSize, &err);
    M_AssertEq(FabricErrorTemplates::kInternalError, err, "server->RdmaWrite(1)");
    err = nullptr; // We have to reset the error by ourselves
    server->Send(clientAddress, messageId, dummyData, kDummyDataSize, &err);
    M_AssertEq(nullptr, err, "server->Send(1)");

    // Simulate correct memory details
    server->RecvAny(&clientAddress, &messageId, &request, sizeof(request), &err);
    M_AssertEq(nullptr, err, "server->RecvAny(2)");
    M_AssertEq(2, messageId);
    server->RdmaWrite(clientAddress, (MemoryRegionDetails*)&request.substream, rdmaBuffer, kRdmaSize, &err);
    M_AssertEq(nullptr, err, "server->RdmaWrite(2)");
    server->Send(clientAddress, messageId, dummyData, kDummyDataSize, &err);
    M_AssertEq(nullptr, err, "server->Send(2)");

    // Simulate old (unregistered) memory details
    GenericRequestHeader request2{};
    server->RecvAny(&clientAddress, &messageId, &request2, sizeof(request2), &err);
    M_AssertEq(nullptr, err, "server->RecvAny(3)");
    M_AssertEq(3, messageId);
    server->RdmaWrite(clientAddress, (MemoryRegionDetails*)&request.substream, rdmaBuffer, kRdmaSize, &err);
    M_AssertEq(FabricErrorTemplates::kInternalError, err, "server->RdmaWrite(3)");

    std::cout << "[SERVER] Waiting for client to finish" << std::endl;
    clientIsDoneFuture.get();
    serverIsDone.set_value();
}

void ClientThread() {
    Error err;

    auto factory = GenerateDefaultFabricFactory();

    auto client = factory->CreateClient(&err);
    M_AssertEq(nullptr, err, "factory->CreateClient");

    auto serverAddress = client->AddServerAddress("127.0.0.1:1816", &err);
    M_AssertEq(nullptr, err, "client->AddServerAddress");

    char* actualRdmaBuffer = new char[kRdmaSize];
    char* dummyData = new char[512];

    GenericRequestHeader request{};
    FabricMessageId messageId = 1;
    strcpy(request.message, "Hello World");

    // Scoped MemoryRegion
    {
        auto mr = client->ShareMemoryRegion(actualRdmaBuffer, kRdmaSize, &err);
        M_AssertEq(nullptr, err, "client->ShareMemoryRegion");
        memcpy(request.substream, mr->GetDetails(), sizeof(MemoryRegionDetails));

        // Simulate faulty memory details
        ((MemoryRegionDetails*)(&request.substream))->key++;
        client->Send(serverAddress, messageId, &request, sizeof(request), &err);
        M_AssertEq(nullptr, err, "client->Send(1)");
        client->Recv(serverAddress, messageId, dummyData, kDummyDataSize, &err);
        M_AssertEq(nullptr, err, "client->Recv(1)");
        messageId++;

        // Simulate correct memory details
        memcpy(request.substream, mr->GetDetails(), sizeof(MemoryRegionDetails));
        client->Send(serverAddress, messageId, &request, sizeof(request), &err);
        M_AssertEq(nullptr, err, "client->Send(2)");
        client->Recv(serverAddress, messageId, dummyData, kDummyDataSize, &err);
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
    std::thread serverThread(ServerMasterThread);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    ClientThread();

    std::cout << "Done testing. Joining server" << std::endl;
    serverThread.join();

    return 0;
}
