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

constexpr size_t kRdmaSize = 5 * 1024 * 1024;

void ServerMasterThread() {
    Error err;
    auto log = CreateDefaultLoggerBin("AutomaticTesting");

    auto factory = GenerateDefaultFabricFactory();
    auto server = factory->CreateAndBindServer(log.get(), "127.0.0.1", 1816, &err);
    M_AssertEq(nullptr, err, "factory->CreateAndBindServer");

    GenericRequestHeader request{};

    FabricAddress clientAddress;
    FabricMessageId messageId;
    server->RecvAny(&clientAddress, &messageId, &request, sizeof(request), &err);
    M_AssertEq(nullptr, err, "server->RecvAny");
    M_AssertEq(123, messageId);
    M_AssertEq("Hello World", request.message);

    char* rdmaBuffer = new char[kRdmaSize];

    server->RdmaWrite(clientAddress, (MemoryRegionDetails*)&request.substream, rdmaBuffer, kRdmaSize, &err);
    M_AssertEq(FabricErrorTemplates::kInternalError, err, "server->RdmaWrite");

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

    auto mr = client->ShareMemoryRegion(actualRdmaBuffer, kRdmaSize, &err);
    M_AssertEq(nullptr, err, "client->ShareMemoryRegion");

    GenericRequestHeader request{};
    strcpy(request.message, "Hello World");
    memcpy(request.substream, mr->GetDetails(), sizeof(MemoryRegionDetails));

    // Simulate faulty data
    ((MemoryRegionDetails*)(&request.substream))->key = ((MemoryRegionDetails*)(&request.substream))->key + 1;
    FabricMessageId messageId = 123;
    client->Send(serverAddress, messageId, &request, sizeof(request), &err);
    M_AssertEq(nullptr, err, "client->Send");

    clientIsDone.set_value();
    std::cout << "[Client] Waiting for server to finish" << std::endl;
    serverIsDoneFuture.wait();
}

int main(int argc, char* argv[]) {
    std::thread serverThread(ServerMasterThread);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    ClientThread();

    std::cout << "Done testing. Joining server" << std::endl;
    serverThread.join();
}
