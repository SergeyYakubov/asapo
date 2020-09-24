#include "fabric_factory_impl.h"
#include "fabric_internal_error.h"
#include "client/fabric_client_impl.h"
#include "server/fabric_server_impl.h"
#include <rdma/fabric.h>

using namespace asapo::fabric;

std::string fi_version_string(uint32_t version) {
    return std::to_string(FI_MAJOR(version)) + "." + std::to_string(FI_MINOR(version));
}

bool FabricFactoryImpl::HasValidVersion(Error* error) const {
    auto current_version = gffm().fi_version();

    if (FI_VERSION_LT(current_version, FabricContextImpl::kMinExpectedLibFabricVersion)) {
        std::string found_version_str = fi_version_string(current_version);
        std::string expected_version_str = fi_version_string(FabricContextImpl::kMinExpectedLibFabricVersion);

        std::string errorText = "Found " + found_version_str + " but expected at least " + expected_version_str;
        *error = FabricErrorTemplates::kOutdatedLibraryError.Generate(errorText);
        return false;
    }

    return true;
}

std::unique_ptr<FabricServer>
FabricFactoryImpl::CreateAndBindServer(const AbstractLogger* logger, const std::string& host, uint16_t port,
                                       Error* error) const {
    if (!HasValidVersion(error)) {
        return nullptr;
    }

    auto server = new FabricServerImpl(logger);

    server->InitAndStartServer(host, port, error);

    return std::unique_ptr<FabricServer>(server);
}

std::unique_ptr<FabricClient>
FabricFactoryImpl::CreateClient(Error* error) const {
    if (!HasValidVersion(error)) {
        return nullptr;
    }

    return std::unique_ptr<FabricClient>(new FabricClientImpl());
}
