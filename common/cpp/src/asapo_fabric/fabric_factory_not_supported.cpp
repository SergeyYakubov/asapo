#include "fabric_factory_not_supported.h"
#include "fabric_error.h"

using namespace asapo::fabric;

std::unique_ptr<FabricServer> asapo::fabric::FabricFactoryNotSupported::CreateAndBindServer(uint16_t port,
        Error* error) const {
    *error = FabricErrorTemplates::kNotSupportedOnBuildError.Generate();
    return nullptr;
}

std::unique_ptr<FabricClient> asapo::fabric::FabricFactoryNotSupported::CreateClient(Error* error) const {
    *error = FabricErrorTemplates::kNotSupportedOnBuildError.Generate();
    return nullptr;
}
