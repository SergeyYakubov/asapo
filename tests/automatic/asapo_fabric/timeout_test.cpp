#include <iostream>
#include <future>
#include <common/error.h>
#include <logger/logger.h>
#include <testing.h>
#include <asapo_fabric/asapo_fabric.h>

using namespace asapo;
using namespace fabric;

void ServerMasterThread() {
    Error err;
    auto log = CreateDefaultLoggerBin("AutomaticTesting");

    auto factory = GenerateDefaultFabricFactory();

    auto server = factory->CreateAndBindServer(log.get(), "127.0.0.1", 1816, &err);
    M_AssertEq(nullptr, err, "factory->CreateAndBindServer");

    // Wait for client to send a request and then shutdown the server
    int dummyBuffer;
    FabricAddress clientAddress;
    FabricMessageId messageId;
    server->RecvAny(&clientAddress, &messageId, &dummyBuffer, sizeof(dummyBuffer), &err);
    M_AssertEq(nullptr, err, "server->RecvAny");
}

void ClientThread() {
    Error err;

    auto factory = GenerateDefaultFabricFactory();

    auto client = factory->CreateClient(&err);
    M_AssertEq(nullptr, err, "factory->CreateClient");

    auto serverAddress = client->AddServerAddress("127.0.0.1:1816", &err);
    M_AssertEq(nullptr, err, "client->AddServerAddress");

    int dummyBuffer;
    client->Send(serverAddress, 1, &dummyBuffer, sizeof(dummyBuffer), &err);
    M_AssertEq(nullptr, err, "client->Send");

    // The server should shut down now!
    client->Recv(serverAddress, 1, &dummyBuffer, sizeof(dummyBuffer), &err);
    M_AssertEq(FabricErrorTemplates::kTimeout, err, "client->Recv");
}

int main(int argc, char* argv[]) {
    std::thread serverThread(ServerMasterThread);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    ClientThread();

    std::cout << "Done testing. Joining server" << std::endl;
    serverThread.join();

    return 0;
}
