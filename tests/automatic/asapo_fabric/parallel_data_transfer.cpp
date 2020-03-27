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
constexpr int kServerThreads = 2;
constexpr int kEachInstanceRuns = 10;
constexpr int kClientThreads = 4;

void ServerChildThread(FabricServer* server, std::atomic<int>* serverTotalRequests, char* expectedRdmaBuffer) {
    constexpr int maxRuns = kClientThreads * kEachInstanceRuns;
    Error err;

    while ((*serverTotalRequests)++ < maxRuns) {
        GenericRequestHeader request{};

        FabricAddress clientAddress;
        FabricMessageId messageId;
        server->RecvAny(&clientAddress, &messageId, &request, sizeof(request), &err);
        M_AssertEq(nullptr, err, "server->RecvAny");
        M_AssertEq("Hello World", request.message);
        M_AssertEq(messageId / kEachInstanceRuns, request.data_id); // is client index
        M_AssertEq(messageId % kEachInstanceRuns, request.data_size); // is client run

        server->RdmaWrite(clientAddress, (MemoryRegionDetails*)&request.substream, expectedRdmaBuffer, kRdmaSize, &err);
        M_AssertEq(nullptr, err, "server->RdmaWrite");

        GenericNetworkResponse response{};
        strcpy(response.message, "Hey, I am the Server");
        server->Send(clientAddress, messageId, &response, sizeof(response), &err);
        M_AssertEq(nullptr, err, "server->Send");
    }

    std::cerr << "A Server is done" << std::endl;
}

void ServerMasterThread(char* expectedRdmaBuffer) {
    Error err;
    auto log = CreateDefaultLoggerBin("AutomaticTesting");

    auto factory = GenerateDefaultFabricFactory();
    auto server = factory->CreateAndBindServer(log.get(), "127.0.0.1", 1816, &err);
    M_AssertEq(nullptr, err, "factory->CreateAndBindServer");
    std::atomic<int> serverTotalRequests;

    std::thread threads[kServerThreads];
    for (auto& thread : threads) {
        thread = std::thread(ServerChildThread, server.get(), &serverTotalRequests, expectedRdmaBuffer);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cerr << "[SERVER] Waiting for all client to finish" << std::endl;
    clientIsDoneFuture.get();
    serverIsDone.set_value();
}

void ClientChildThread(int index, char* expectedRdmaBuffer) {
    auto factory = GenerateDefaultFabricFactory();
    Error err;

    auto client = factory->CreateClient(&err);
    M_AssertEq(nullptr, err, "factory->CreateClient");

    auto serverAddress = client->AddServerAddress("127.0.0.1:1816", &err);
    M_AssertEq(nullptr, err, "client->AddServerAddress");

    char* actualRdmaBuffer = new char[kRdmaSize];

    auto mr = client->ShareMemoryRegion(actualRdmaBuffer, kRdmaSize, &err);
    M_AssertEq(nullptr, err, "client->ShareMemoryRegion");

    for (int run = 0; run < kEachInstanceRuns; run++) {
        std::cerr << "Client run: " << run << std::endl;

        GenericRequestHeader request{};
        strcpy(request.message, "Hello World");
        memcpy(request.substream, mr->GetDetails(), sizeof(MemoryRegionDetails));
        request.data_id = index;
        request.data_size = run;
        FabricMessageId messageId = (index * kEachInstanceRuns) + run;
        client->Send(serverAddress, messageId, &request, sizeof(request), &err);
        M_AssertEq(nullptr, err, "client->Send");

        GenericNetworkResponse response{};
        client->Recv(serverAddress, messageId, &response, sizeof(response), &err);
        M_AssertEq(nullptr, err, "client->Recv");
        M_AssertEq("Hey, I am the Server", response.message);

        for (size_t i = 0; i < kRdmaSize; i++) {
            // Check to reduce log spam
            if (expectedRdmaBuffer[i] != actualRdmaBuffer[i]) {
                M_AssertEq(expectedRdmaBuffer[i], actualRdmaBuffer[i],
                           "Expect rdma[i] == acutal[i], i = " + std::to_string(i));
            }
        }
    }
    std::cout << "A Client is done" << std::endl;
}

void ClientMasterThread(char* expectedRdmaBuffer) {
    std::thread threads[kClientThreads];
    for (int i = 0; i < kClientThreads; i++) {
        threads[i] = std::thread(ClientChildThread, i, expectedRdmaBuffer);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    clientIsDone.set_value();
    std::cout << "[Client] Waiting for server to finish" << std::endl;
    serverIsDoneFuture.wait();
}

int main(int argc, char* argv[]) {

    std::cout << "Client is writing to std::cout" << std::endl;
    std::cerr << "Server is writing to std::cerr" << std::endl;

    char* expectedRdmaBuffer = new char[kRdmaSize];
    for (size_t i = 0; i < kRdmaSize; i++) {
        expectedRdmaBuffer[i] = (char)i;
    }

    std::thread serverMasterThread(ServerMasterThread, expectedRdmaBuffer);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    ClientMasterThread(expectedRdmaBuffer);

    std::cout << "Done testing. Joining server" << std::endl;
    serverMasterThread.join();
}
