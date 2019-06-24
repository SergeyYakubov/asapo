#ifndef ASAPO_SYSTEM__IO_ERROR_H
#define ASAPO_SYSTEM__IO_ERROR_H

#include "common/error.h"

namespace asapo {


enum class IOErrorType {
    kUnknownIOError,
    kBadFileNumber,
    kResourceTemporarilyUnavailable,
    kFileNotFound,
    kReadError,
    kPermissionDenied,
    kUnsupportedAddressFamily,
    kInvalidAddressFormat,
    kAddressAlreadyInUse,
    kConnectionRefused,
    kConnectionResetByPeer,
    kTimeout,
    kFileAlreadyExists,
    kNoSpaceLeft,
    kSocketOperationOnNonSocket,
    kInvalidMemoryAddress,
    kUnableToResolveHostname,
    kSocketOperationUnknownAtLevel,
    kSocketOperationValueOutOfBound,
    kAddressNotValid,
    kBrokenPipe

};

using IOError = ServiceError<IOErrorType, ErrorType::kIOError>;
using IOErrorTemplate = ServiceErrorTemplate<IOErrorType, ErrorType::kIOError>;

namespace IOErrorTemplates {
auto const kUnknownIOError = IOErrorTemplate {
    "Unknown Error", IOErrorType::kUnknownIOError
};

auto const kFileNotFound = IOErrorTemplate {
    "No such file or directory", IOErrorType::kFileNotFound
};
auto const kReadError = IOErrorTemplate {
    "Read error", IOErrorType::kReadError
};
auto const kBadFileNumber = IOErrorTemplate {
    "Bad file number", IOErrorType::kBadFileNumber
};
auto const kResourceTemporarilyUnavailable = IOErrorTemplate {
    "Resource temporarily unavailable", IOErrorType::kResourceTemporarilyUnavailable
};
auto const kPermissionDenied = IOErrorTemplate {
    "Permission denied", IOErrorType::kPermissionDenied
};
auto const kUnsupportedAddressFamily = IOErrorTemplate {
    "Unsupported address family", IOErrorType::kUnsupportedAddressFamily
};
auto const kInvalidAddressFormat = IOErrorTemplate {
    "Invalid address format", IOErrorType::kInvalidAddressFormat
};
auto const kAddressAlreadyInUse = IOErrorTemplate {
    "Address already in use", IOErrorType::kAddressAlreadyInUse
};
auto const kConnectionRefused = IOErrorTemplate {
    "Connection refused", IOErrorType::kConnectionRefused
};
auto const kConnectionResetByPeer = IOErrorTemplate {
    "kConnectionResetByPeer", IOErrorType::kConnectionResetByPeer
};
auto const kTimeout = IOErrorTemplate {
    "timeout", IOErrorType::kTimeout
};
auto const kFileAlreadyExists = IOErrorTemplate {
    "kFileAlreadyExists", IOErrorType::kFileAlreadyExists
};
auto const kNoSpaceLeft = IOErrorTemplate {
    "kNoSpaceLeft", IOErrorType::kNoSpaceLeft
};
auto const kSocketOperationOnNonSocket = IOErrorTemplate {
    "kSocketOperationOnNonSocket", IOErrorType::kSocketOperationOnNonSocket
};
auto const kInvalidMemoryAddress = IOErrorTemplate {
    "kInvalidMemoryAddress", IOErrorType::kInvalidMemoryAddress
};
auto const kUnableToResolveHostname = IOErrorTemplate {
    "kUnableToResolveHostname", IOErrorType::kUnableToResolveHostname
};
auto const kSocketOperationUnknownAtLevel =  IOErrorTemplate {
    "kSocketOperationUnknownAtLevel", IOErrorType::kSocketOperationUnknownAtLevel
};

auto const kSocketOperationValueOutOfBound =  IOErrorTemplate {
    "kSocketOperationValueOutOfBound", IOErrorType::kSocketOperationValueOutOfBound
};

auto const kAddressNotValid =  IOErrorTemplate {
    "Address not valid", IOErrorType::kAddressNotValid
};

auto const kBrokenPipe =  IOErrorTemplate {
    "Broken pipe/connection", IOErrorType::kBrokenPipe
};


}

}

#endif //ASAPO_SYSTEM__IO_ERROR_H
