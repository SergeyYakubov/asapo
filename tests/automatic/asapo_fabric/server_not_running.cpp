#include <common/error.h>
#include <asapo_fabric/asapo_fabric.h>
#include <testing.h>

using namespace asapo;
using namespace fabric;

int main(int argc, char* argv[]) {
    Error err;
    auto factory = GenerateDefaultFabricFactory();

    auto client = factory->CreateClient(&err);
    M_AssertEq(nullptr, err, "factory->CreateClient");

    auto serverAddress = client->AddServerAddress("127.0.0.1:1234", &err);
    M_AssertEq(FabricErrorTemplates::kConnectionRefusedError, err, "client->AddServerAddress");

    return 0;
}
