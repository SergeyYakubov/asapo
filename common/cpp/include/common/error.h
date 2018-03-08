#ifndef HIDRA2_ERROR_H
#define HIDRA2_ERROR_H

#include <string>
#include <memory>

namespace hidra2 {

enum class ErrorType {
    kHidraError,
    kHttpError,
    kUnknownError,

    kBadFileNumber,
    kResourceTemporarilyUnavailable,
    kFileNotFound,
    kReadError,
    kPermissionDenied,
    kUnsupportedAddressFamily,
    kInvalidAddressFormat,
    kEndOfFile,
    kAddressAlreadyInUse,
    kConnectionRefused,
    kConnectionResetByPeer,
    kTimeout,
    kFileAlreadyExists,
    kNoSpaceLeft,
    kSocketOperationOnNonSocket,
    kMemoryAllocationError,
    kInvalidMemoryAddress,
    kUnableToResolveHostname,
};

class ErrorInterface;

// Is nullptr if no error is set
using Error = std::unique_ptr<ErrorInterface>;

class ErrorInterface {
  public:
    virtual std::string Explain() const noexcept = 0;
    virtual void Append(const std::string& value) noexcept = 0;
    virtual ErrorType GetErrorType() const noexcept = 0;
    virtual ~ErrorInterface() = default; // needed for unique_ptr to delete itself
};

class SimpleError: public ErrorInterface {
  private:
    std::string error_;
    ErrorType error_type_ = ErrorType::kHidraError;
  public:
    explicit SimpleError(const std::string& error): error_{error} {

    }
    SimpleError(const std::string& error, ErrorType error_type ): error_{error}, error_type_{error_type} {
    }

    void Append(const std::string& value) noexcept override {
        error_ += ": " + value;
    }

    std::string Explain() const noexcept override  {
        return error_;
    }

    ErrorType GetErrorType() const noexcept override  {
        return error_type_;
    }
};

class ErrorTemplate {
  private:
    std::string error_;
    ErrorType error_type_ = ErrorType::kHidraError;
  public:
    explicit ErrorTemplate(const std::string& error): error_{error} {

    }
    ErrorTemplate(const std::string& error, ErrorType error_type ): error_{error}, error_type_{error_type} {
    }

    inline Error Copy() const {
        return Error(new SimpleError{error_, error_type_});
    }
};

inline Error TextError(const std::string& error) {
    return Error{new SimpleError{error}};
}

inline Error TextErrorWithType(const std::string& error, ErrorType error_type) {
    return Error{new SimpleError{error, error_type}};
}


}
#endif //HIDRA2_ERROR_H
