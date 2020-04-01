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

constexpr int kTotalRuns = 3;
constexpr int kEachInstanceRuns = 5;
constexpr size_t kRdmaSize = 5 * 1024 * 1024;

void ServerMasterThread(char* expectedRdmaBuffer) {
    Error err;
    auto log = CreateDefaultLoggerBin("AutomaticTesting");

    auto factory = GenerateDefaultFabricFactory();
    auto server = factory->CreateAndBindServer(log.get(), "127.0.0.1", 1816, &err);
    M_AssertEq(nullptr, err, "factory->CreateAndBindServer");

    for (int run = 0; run < kTotalRuns; run++) {
        for (int instanceRuns = 0; instanceRuns < kEachInstanceRuns; instanceRuns++) {
            GenericRequestHeader request{};

            FabricAddress clientAddress;
            FabricMessageId messageId;
            server->RecvAny(&clientAddress, &messageId, &request, sizeof(request), &err);
            M_AssertEq(nullptr, err, "server->RecvAny");
            M_AssertEq(123 + instanceRuns, messageId);
            M_AssertEq("Hello World", request.message);

            server->RdmaWrite(clientAddress, (MemoryRegionDetails*) &request.substream, expectedRdmaBuffer, kRdmaSize,
                              &err);
            M_AssertEq(nullptr, err, "server->RdmaWrite");

            GenericNetworkResponse response{};
            strcpy(response.message, "Hey, I am the Server");
            server->Send(clientAddress, messageId, &response, sizeof(response), &err);
            M_AssertEq(nullptr, err, "server->Send");
        }
    }

    std::cout << "[SERVER] Waiting for client to finish" << std::endl;
    clientIsDoneFuture.get();
    serverIsDone.set_value();
}

void ClientThread(char* expectedRdmaBuffer) {
    Error err;

    for (int run = 0; run < kTotalRuns; run++) {
        std::cout << "Running client " << run << std::endl;
        for (int instanceRuns = 0; instanceRuns < kEachInstanceRuns; instanceRuns++) {
            auto factory = GenerateDefaultFabricFactory();

            auto client = factory->CreateClient(&err);
            M_AssertEq(nullptr, err, "factory->CreateClient");

            auto serverAddress = client->AddServerAddress("127.0.0.1:1816", &err);
            M_AssertEq(nullptr, err, "client->AddServerAddress");

            auto actualRdmaBuffer = std::unique_ptr<char[]>(new char[kRdmaSize]);

            auto mr = client->ShareMemoryRegion(actualRdmaBuffer.get(), kRdmaSize, &err);
            M_AssertEq(nullptr, err, "client->ShareMemoryRegion");

            GenericRequestHeader request{};
            strcpy(request.message, "Hello World");
            memcpy(request.substream, mr->GetDetails(), sizeof(MemoryRegionDetails));
            FabricMessageId messageId = 123 + instanceRuns;
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
    }
    clientIsDone.set_value();
    serverIsDoneFuture.get();
}

int main(int argc, char* argv[]) {
    auto expectedRdmaBuffer = std::unique_ptr<char[]>(new char[kRdmaSize]);;
    for (size_t i = 0; i < kRdmaSize; i++) {
        expectedRdmaBuffer[i] = (char)i;
    }

    std::thread serverThread(ServerMasterThread, expectedRdmaBuffer.get());

    std::this_thread::sleep_for(std::chrono::seconds(2));
    ClientThread(expectedRdmaBuffer.get());

    std::cout << "Done testing. Joining server" << std::endl;
    serverThread.join();

    return 0;
}
