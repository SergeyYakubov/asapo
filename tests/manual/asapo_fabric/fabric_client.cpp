#include <asapo_fabric/asapo_fabric.h>
#include <iostream>
#include <common/data_structs.h>
#include <common/networking.h>

using namespace asapo;
using namespace asapo::fabric;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout
                << "Usage: " << argv[0] << " <serverAddress> <serverPort>" << std::endl
                << "If the address is localhost or 127.0.0.1 the verbs connection will be emulated" << std::endl
                ;
        return 1;
    }

    std::string serverAddressString = std::string(argv[1]) + ':' + std::string(argv[2]);

    Error error;
    auto factory = GenerateDefaultFabricFactory();

    auto client = factory->CreateClient(&error);
    if (error) {
        std::cout << "Client exited with error: " << error << std::endl;
        return 1;
    }

    size_t dataBufferSize = 1024 * 1024 * 400 /*400 MiByte*/;
    FileData dataBuffer = FileData{new uint8_t[dataBufferSize]};

    auto serverAddress = client->AddServerAddress(serverAddressString, &error);
    if (error) {
        std::cout << "Client exited with error: " << error << std::endl;
        return 1;
    }
    std::cout << "Added server address" << std::endl;

    auto mr = client->ShareMemoryRegion(dataBuffer.get(), dataBufferSize, &error);
    if (error) {
        std::cout << "Client exited with error: " << error << std::endl;
        return 1;
    }

    uint64_t totalTransferSize = 0;
    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "Starting message loop" << std::endl;
    for (FabricMessageId messageId = 0; messageId < 10 && !error; messageId++) {
        GenericRequestHeader request{};
        memcpy(&request.message, mr->GetDetails(), sizeof(MemoryRegionDetails));
        client->Send(serverAddress, messageId, &request, sizeof(request), &error);
        if (error) {
            break;
        }

        GenericNetworkResponse response{};
        client->Recv(serverAddress, messageId, &response, sizeof(response), &error);
        if (error) {
            break;
        }

        if (strcmp((char*)dataBuffer.get(), "I (the server) wrote into your buffer.") != 0) {
            error = TextError("The buffer was not written with the expected text");
            break;
        }
        memset(dataBuffer.get(), 0, 64);

        totalTransferSize += dataBufferSize;
    }
    auto end = std::chrono::high_resolution_clock::now();

    if (error) {
        std::cout << "Client exited with error: " << error << std::endl;
        return 1;
    }

    auto timeTook = end - start;
    std::cout << "Transferred " << (((totalTransferSize) / 1024) / 1024) << " MiBytes in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(timeTook).count() << "ms" << std::endl;

    return 0;
}