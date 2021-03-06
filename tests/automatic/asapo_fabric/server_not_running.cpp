#include <asapo/common/error.h>
#include <asapo/asapo_fabric/asapo_fabric.h>
#include <testing.h>
#include <iostream>

using namespace asapo;
using namespace fabric;

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

    Error err;
    auto factory = GenerateDefaultFabricFactory();

    auto client = factory->CreateClient(&err);
    M_AssertEq(nullptr, err, "factory->CreateClient");

    client->AddServerAddress(hostname + ":" + std::to_string(port), &err);
    M_AssertEq(FabricErrorTemplates::kConnectionRefusedError, err, "client->AddServerAddress");

    return 0;
}
