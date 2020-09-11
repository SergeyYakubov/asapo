#ifndef ASAPO_FABRIC_ERROR_H
#define ASAPO_FABRIC_ERROR_H

namespace asapo {
namespace fabric {
enum class FabricErrorType {
    kNotSupported,
    kLibraryNotFound,
    kLibraryCompatibilityError,
    kLibraryOutdated,
    kInternalError, // An error that was produced by LibFabric
    kInternalOperationCanceled, // An error that was produced by LibFabric
    kInternalConnectionError, // This might occur when the connection is unexpectedly closed
    kNoDeviceFound,
    kClientNotInitialized,
    kConnectionRefused,
};


using FabricError = ServiceError<FabricErrorType, ErrorType::kFabricError>;
using FabricErrorTemplate = ServiceErrorTemplate<FabricErrorType, ErrorType::kFabricError>;

namespace FabricErrorTemplates {
auto const kNotSupportedOnBuildError = FabricErrorTemplate {
        "This build of ASAPO does not support LibFabric", FabricErrorType::kNotSupported
};
auto const kLibraryNotFoundError = FabricErrorTemplate {
        "asapo-fabric, LibFabric or dependencies were not found", FabricErrorType::kLibraryNotFound
};
auto const kLibraryCompatibilityError = FabricErrorTemplate {
        "LibFabric was found but somehow some a function is missing", FabricErrorType::kLibraryCompatibilityError
};
auto const kOutdatedLibraryError = FabricErrorTemplate {
    "LibFabric outdated", FabricErrorType::kLibraryOutdated
};
auto const kInternalError = FabricErrorTemplate {
    "Internal LibFabric error", FabricErrorType::kInternalError
};
auto const kInternalOperationCanceledError = FabricErrorTemplate {
    "Internal LibFabric operation canceled error", FabricErrorType::kInternalOperationCanceled
};
auto const kNoDeviceFoundError = FabricErrorTemplate {
    "No device was found (Check your config)", FabricErrorType::kNoDeviceFound
};
auto const kClientNotInitializedError = FabricErrorTemplate {
    "The client was not initialized. Add server address first!",
    FabricErrorType::kClientNotInitialized
};
auto const kConnectionRefusedError = FabricErrorTemplate {
    "Connection refused",
    FabricErrorType::kConnectionRefused
};
auto const kInternalConnectionError = FabricErrorTemplate {
    "Connection error (maybe a disconnect?)",
    FabricErrorType::kInternalConnectionError
};
}

}
}
#endif //ASAPO_FABRIC_ERROR_H
