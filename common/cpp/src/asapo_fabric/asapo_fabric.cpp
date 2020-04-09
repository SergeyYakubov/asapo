#include <asapo_fabric/asapo_fabric.h>

#ifdef LIBFABRIC_ENABLED
#include "fabric_factory_impl.h"
#else
#include "fabric_factory_not_supported.h"
#endif

using namespace asapo::fabric;

std::unique_ptr<FabricFactory> asapo::fabric::GenerateDefaultFabricFactory() {
#ifdef LIBFABRIC_ENABLED
    return std::unique_ptr<FabricFactory>(new FabricFactoryImpl());
#else
    return std::unique_ptr<FabricFactory>(new FabricFactoryNotSupported());
#endif
}
