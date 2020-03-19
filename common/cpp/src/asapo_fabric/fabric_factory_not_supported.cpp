#include "fabric_factory_not_supported.h"

using namespace asapo::fabric;

std::unique_ptr<FabricServer> asapo::fabric::FabricFactoryNotSupported::CreateAndBindServer(Error* error) const {
    *error = TextError("This build of ASAPO does not support LibFabric.");
    return nullptr;
}

std::unique_ptr<FabricClient> asapo::fabric::FabricFactoryNotSupported::CreateClient(Error* error) const {
    *error = TextError("This build of ASAPO does not support LibFabric.");
    return nullptr;
}

