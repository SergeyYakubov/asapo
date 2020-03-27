
#include <common/error.h>
#include <logger/logger.h>
#include <asapo_fabric/asapo_fabric.h>
#include <iostream>
#include <thread>

using namespace asapo;
using namespace fabric;

constexpr int kRequestThreads = 2;

void ServerRequestThread(FabricServer* server, int i) {
    Error err;
    FabricAddress address;
    FabricMessageId messageId;
    int dummyBuffer;
    server->RecvAny(&address, &messageId, &dummyBuffer, sizeof(dummyBuffer), &err);

    std::cerr << "server " << i << " RecvAny " << err << " messageId: " << messageId << " address: " << address << std::endl;
}

void ServerThread() {
    Error err;
    auto log = CreateDefaultLoggerBin("AutomaticTesting");

    auto factory = GenerateDefaultFabricFactory();
    auto server = factory->CreateAndBindServer(log.get(), "127.0.0.1", 1816, &err);

    std::thread requestThreads[kRequestThreads];
    for (int i = 0; i < kRequestThreads; i++) {
        requestThreads[i] = std::thread(ServerRequestThread, server.get(), i);
    }
    for (auto& thread : requestThreads) {
        thread.join();
    }


}

void ClientThread() {
    Error err;
    auto factory = GenerateDefaultFabricFactory();
    auto client = factory->CreateClient(&err);

    auto serverAddress = client->AddServerAddress("127.0.0.1:1816", &err);

    int dummyBuffer = 0;
    client->Send(serverAddress, 1, &dummyBuffer, sizeof(dummyBuffer), &err);
    std::cout << "client->Send1 " << err << std::endl;
}

int main() {

    auto serverThread = std::thread(ServerThread);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    ClientThread();

    serverThread.join();

    return 0;
}
