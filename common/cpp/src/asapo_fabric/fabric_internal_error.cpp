#include "fabric_internal_error.h"
#include <rdma/fi_errno.h>
#include <asapo_fabric/fabric_error.h>

asapo::Error asapo::fabric::ErrorFromFabricInternal(const std::string& where, int internalStatusCode) {
    std::string errText = where + ": " + fi_strerror(-internalStatusCode);
    switch (-internalStatusCode) {
    case FI_ECANCELED:
        return FabricErrorTemplates::kInternalOperationCanceledError.Generate(errText);
    case FI_EIO:
        return FabricErrorTemplates::kInternalConnectionError.Generate(errText);
    }
    return FabricErrorTemplates::kInternalError.Generate(errText);
}
