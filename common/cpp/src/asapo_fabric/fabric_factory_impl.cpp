#include "fabric_factory_impl.h"
#include "fabric_internal_impl_common.h"
#include <rdma/fabric.h>

using namespace asapo::fabric;

std::string fi_version_string(uint32_t version) {
    return std::to_string(FI_MAJOR(version)) + "." + std::to_string(FI_MINOR(version));
}

bool FabricFactoryImpl::HasValidVersion(Error* error) const {
    auto current_version = fi_version();

    if (FI_VERSION_LT(current_version, EXPECTED_FI_VERSION)) {
        std::string found_version_str = fi_version_string(current_version);
        std::string expected_version_str = fi_version_string(EXPECTED_FI_VERSION);

        std::string errorText = "LibFabric outdated.";
        errorText += " (Found " + found_version_str + " but expected at least " + expected_version_str + ")";

        *error = TextError(errorText);
        return false;
    }

    return true;
}

std::unique_ptr<FabricClient>
FabricFactoryImpl::CreateClient(Error* error) const {
    if (!HasValidVersion(error)) {
        return nullptr;
    }

    *error = TextError("This build of ASAPO does not support LibFabric.");
    return nullptr;
}

std::unique_ptr<FabricServer> FabricFactoryImpl::CreateAndBindServer(Error* error) const {
    if (!HasValidVersion(error)) {
        return nullptr;
    }

    *error = TextError("This build of ASAPO does not support LibFabric.");
    return nullptr;
}
