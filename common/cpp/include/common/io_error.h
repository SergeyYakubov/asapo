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
    kAddressNotValid

};

class IOError : public SimpleError {
  private:
    IOErrorType io_error_type_;
  public:
    IOError(const std::string& error, IOErrorType io_error_type) : SimpleError(error, ErrorType::kIOError) {
        io_error_type_ = io_error_type;
    }

    IOErrorType GetIOErrorType() const noexcept {
        return io_error_type_;
    }
};

class IOErrorTemplate : public SimpleErrorTemplate {
  protected:
    IOErrorType io_error_type_;
  public:
    IOErrorTemplate(const std::string& error, IOErrorType io_error_type) : SimpleErrorTemplate(error, ErrorType::kIOError) {
        io_error_type_ = io_error_type;
    }

    inline IOErrorType GetIOErrorType() const noexcept {
        return io_error_type_;
    }

    inline Error Generate() const noexcept override {
        return Error(new IOError(error_, io_error_type_));
    }

    inline bool operator == (const Error& rhs) const override {
        return SimpleErrorTemplate::operator==(rhs)
               && GetIOErrorType() == ((IOError*)rhs.get())->GetIOErrorType();
    }
};

static inline std::ostream& operator<<(std::ostream& os, const IOErrorTemplate& err) {
    return os << err.Text();
}


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
    "kTimeout", IOErrorType::kTimeout
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

}

}

#endif //ASAPO_SYSTEM__IO_ERROR_H
