#include <asapo_fabric/asapo_fabric.h>
#include <iostream>
#include <io/io_factory.h>
#include <common/networking.h>

using namespace asapo;
using namespace asapo::fabric;

volatile bool running = false;

void ServerThread(FabricServer* server, size_t bufferSize, FileData* buffer) {
    Error error;
    while(running && !error) {
        FabricAddress clientAddress;
        FabricMessageId messageId;
        GenericRequestHeader request;

        server->RecvAny(&clientAddress, &messageId, &request, sizeof(request), &error);
        if (error) {
            break;
        }

        std::cout << "Got a request from " << clientAddress << " id: " << messageId << std::endl;
        server->RdmaWrite(clientAddress, (MemoryRegionDetails*)&request.message, buffer->get(), bufferSize, &error);

        GenericNetworkResponse response{};
        server->Send(clientAddress, messageId, &response, sizeof(response), &error);
    }

    if (error) {
        std::cerr << "Server thread exited with an error: " << error << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout
                << "Usage: " << argv[0] << " <listenAddress> <listenPort>" << std::endl
                << "If the address is localhost or 127.0.0.1 the verbs connection will be emulated" << std::endl
                ;
        return 1;
    }

    Error error;
    auto io = GenerateDefaultIO();
    auto factory = GenerateDefaultFabricFactory();
    Logger log = CreateDefaultLoggerBin("FabricTestServer");

    uint16_t port = (uint16_t)strtoul(argv[2], nullptr, 10);
    auto server = factory->CreateAndBindServer(log.get(), argv[1], port, &error);
    if (error) {
        std::cerr << error << std::endl;
        return 1;
    }

    std::cout << "Server is listening on " << server->GetAddress() << std::endl;

    size_t dataBufferSize = 1024*1024*400 /*400 MiByte*/;
    FileData dataBuffer = FileData{new uint8_t[dataBufferSize]};
    strcpy((char*)dataBuffer.get(), "I (the server) wrote into your buffer.");

    running = true;
    auto thread = io->NewThread("ServerThread", [&server, &dataBufferSize, &dataBuffer](){
        ServerThread(server.get(), dataBufferSize, &dataBuffer);
    });

    std::cout << "Press Enter to stop the server." << std::endl;

    getchar();
    std::cout << "Stopping server..." << std::endl;

    running = false;
    thread->join();

    if (error) {
        std::cerr << "Client exited with error: " << error << std::endl;
        return 1;
    }

    return 0;
}
