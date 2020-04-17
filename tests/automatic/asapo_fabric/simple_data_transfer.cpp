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

void ServerMasterThread(const std::string& hostname, uint16_t port, char* expectedRdmaBuffer) {
    {
        Error err;
        auto log = CreateDefaultLoggerBin("AutomaticTesting");

        auto factory = GenerateDefaultFabricFactory();
        auto server = factory->CreateAndBindServer(log.get(), hostname, port, &err);
        M_AssertEq(nullptr, err, "factory->CreateAndBindServer");

        for (int run = 0; run < kTotalRuns; run++) {
            for (int instanceRuns = 0; instanceRuns < kEachInstanceRuns; instanceRuns++) {
                GenericRequestHeader request{};

                FabricAddress clientAddress;
                FabricMessageId messageId;
                // In order to run the tests more stable. Otherwise a timeout could occurred with valgrind
                int tries = 0;
                do {
                    err = nullptr;
                    server->RecvAny(&clientAddress, &messageId, &request, sizeof(request), &err);
                } while (err == IOErrorTemplates::kTimeout && tries++ < 2);
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
    }
    std::cout << "[SERVER] Server is done" << std::endl;
    serverIsDone.set_value();
}

void ClientThread(const std::string& hostname, uint16_t port, char* expectedRdmaBuffer) {
    Error err;

    for (int run = 0; run < kTotalRuns; run++) {
        std::cout << "Running client " << run << std::endl;
        for (int instanceRuns = 0; instanceRuns < kEachInstanceRuns; instanceRuns++) {
            auto factory = GenerateDefaultFabricFactory();

            auto client = factory->CreateClient(&err);
            M_AssertEq(nullptr, err, "factory->CreateClient");

            auto serverAddress = client->AddServerAddress(hostname + ":" + std::to_string(port), &err);
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

    auto expectedRdmaBuffer = std::unique_ptr<char[]>(new char[kRdmaSize]);
    for (size_t i = 0; i < kRdmaSize; i++) {
        expectedRdmaBuffer[i] = (char)i;
    }

    std::thread serverThread(ServerMasterThread, hostname, port, expectedRdmaBuffer.get());

    std::this_thread::sleep_for(std::chrono::seconds(2));
    ClientThread(hostname, port, expectedRdmaBuffer.get());

    std::cout << "Done testing. Joining server" << std::endl;
    serverThread.join();

    return 0;
}
