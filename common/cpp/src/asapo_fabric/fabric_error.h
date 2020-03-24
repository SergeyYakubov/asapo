#ifndef ASAPO_FABRICERRORCONVERTER_H
#define ASAPO_FABRICERRORCONVERTER_H

#include <common/error.h>

namespace asapo { namespace fabric {
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

/**
 * internalStatusCode must be a negative number
 * (Which all libfabric api calls usually return in an error case
 */
Error ErrorFromFabricInternal(const std::string& where, int internalStatusCode);

namespace FabricErrorTemplates {
    auto const kNotSupportedOnBuildError = FabricErrorTemplate {
            "This build of ASAPO does not support LibFabric", FabricErrorType::kNotSupported
    };
    auto const kOutdatedLibraryError = FabricErrorTemplate {
            "LibFabric outdated", FabricErrorType::kOutdatedLibrary
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

}}

#endif //ASAPO_FABRICERRORCONVERTER_H
