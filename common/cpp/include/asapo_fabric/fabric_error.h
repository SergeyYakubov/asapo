#ifndef ASAPO_FABRIC_ERROR_H
#define ASAPO_FABRIC_ERROR_H

namespace asapo {
namespace fabric {
enum class FabricErrorType {
    kNotSupported,
    kOutdatedLibrary,
    kInternalError, // An error that was produced by LibFabric
    kNoDeviceFound,
    kClientNotInitialized,
    kTimeout,
    kConnectionRefused,
};


using FabricError = ServiceError<FabricErrorType, ErrorType::kFabricError>;
using FabricErrorTemplate = ServiceErrorTemplate<FabricErrorType, ErrorType::kFabricError>;

namespace FabricErrorTemplates {
auto const kNotSupportedOnBuildError = FabricErrorTemplate {
    "This build of ASAPO does not support LibFabric", FabricErrorType::kNotSupported
};
auto const kOutdatedLibraryError = FabricErrorTemplate {
        "LibFabric outdated", FabricErrorType::kOutdatedLibrary
};
auto const kInternalError = FabricErrorTemplate {
        "Internal LibFabric Error", FabricErrorType::kInternalError
};
auto const kNoDeviceFoundError = FabricErrorTemplate {
    "No device was found (Check your config)", FabricErrorType::kNoDeviceFound
};
auto const kClientNotInitializedError = FabricErrorTemplate {
    "The client was not initialized. Add server address first!",
    FabricErrorType::kClientNotInitialized
};
auto const kTimeout = FabricErrorTemplate {
    "Timeout",
    FabricErrorType::kTimeout
};
auto const kConnectionRefusedError = FabricErrorTemplate {
    "Connection refused",
    FabricErrorType::kConnectionRefused
};
}

}
}
#endif //ASAPO_FABRIC_ERROR_H
