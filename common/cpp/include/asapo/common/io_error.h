#ifndef ASAPO_SYSTEM__IO_ERROR_H
#define ASAPO_SYSTEM__IO_ERROR_H

#include "error.h"

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
    kBrokenPipe,
    kNotConnected
};

using IOError = ServiceError<IOErrorType>;
using IOErrorTemplate = ServiceErrorTemplate<IOErrorType>;

namespace IOErrorTemplates {
auto const kUnknownIOError = IOErrorTemplate {
    "unknown error", IOErrorType::kUnknownIOError
};

auto const kFileNotFound = IOErrorTemplate {
    "no such file or directory", IOErrorType::kFileNotFound
};
auto const kReadError = IOErrorTemplate {
    "read error", IOErrorType::kReadError
};
auto const kBadFileNumber = IOErrorTemplate {
    "bad file number", IOErrorType::kBadFileNumber
};
auto const kResourceTemporarilyUnavailable = IOErrorTemplate {
    "resource temporarily unavailable", IOErrorType::kResourceTemporarilyUnavailable
};

auto const kPermissionDenied = IOErrorTemplate {
    "permission denied", IOErrorType::kPermissionDenied
};
auto const kUnsupportedAddressFamily = IOErrorTemplate {
    "unsupported address family", IOErrorType::kUnsupportedAddressFamily
};
auto const kInvalidAddressFormat = IOErrorTemplate {
    "invalid address format", IOErrorType::kInvalidAddressFormat
};
auto const kAddressAlreadyInUse = IOErrorTemplate {
    "address already in use", IOErrorType::kAddressAlreadyInUse
};
auto const kConnectionRefused = IOErrorTemplate {
    "connection refused", IOErrorType::kConnectionRefused
};
auto const kNotConnected = IOErrorTemplate {
    "not connected", IOErrorType::kNotConnected
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
    "address not valid", IOErrorType::kAddressNotValid
};

auto const kBrokenPipe =  IOErrorTemplate {
    "broken pipe/connection", IOErrorType::kBrokenPipe
};


}

}

#endif //ASAPO_SYSTEM__IO_ERROR_H
