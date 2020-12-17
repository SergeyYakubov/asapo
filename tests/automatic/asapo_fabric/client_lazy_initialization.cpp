#include <asapo/common/error.h>
#include <asapo/asapo_fabric/asapo_fabric.h>
#include <testing.h>

using namespace asapo;
using namespace fabric;

int main(int argc, char* argv[]) {
    Error err;
    auto factory = GenerateDefaultFabricFactory();

    auto client = factory->CreateClient(&err);
    M_AssertEq(nullptr, err, "factory->CreateClient");

    M_AssertEq("", client->GetAddress());

    int dummyBuffer = 0;
    auto mr = client->ShareMemoryRegion(&dummyBuffer, sizeof(dummyBuffer), &err);
    M_AssertEq(FabricErrorTemplates::kClientNotInitializedError, err, "client->ShareMemoryRegion");
    err = nullptr;

    // Other methods require an serverAddress which initializes the client

    return 0;
}
