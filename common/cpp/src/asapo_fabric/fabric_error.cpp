#include "fabric_error.h"
#include <rdma/fi_errno.h>

asapo::Error asapo::fabric::ErrorFromFabricInternal(const std::string& where, int internalStatusCode) {
    std::string errText = where + ": " + fi_strerror(-internalStatusCode);
    auto err = new ServiceError<FabricErrorType, ErrorType::kFabricError>(errText, FabricErrorType::kInternalError);
    return Error(err);
}
