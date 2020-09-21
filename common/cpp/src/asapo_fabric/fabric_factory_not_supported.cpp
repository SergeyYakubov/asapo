#include "fabric_factory_not_supported.h"

#include <utility>
#include "fabric_internal_error.h"

using namespace asapo::fabric;

FabricFactoryNotSupported::FabricFactoryNotSupported(FabricErrorTemplate reason) : reason_(std::move(reason)) {
}


std::unique_ptr<FabricServer> asapo::fabric::FabricFactoryNotSupported::CreateAndBindServer(
    const AbstractLogger* logger, const std::string& host, uint16_t port,
    Error* error) const {
    *error = reason_.Generate();
    return nullptr;
}

std::unique_ptr<FabricClient> asapo::fabric::FabricFactoryNotSupported::CreateClient(Error* error) const {
    *error = reason_.Generate();
    return nullptr;
}
