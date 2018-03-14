#ifndef HIDRA2_ERROR_H
#define HIDRA2_ERROR_H

#include <string>
#include <memory>
#include <utility>

namespace hidra2 {

enum class ErrorType {
    kUnknownError,

    kHidraError,
    kHttpError,
    kIOError,
    kReceiverError,

    kMemoryAllocationError,
    kEndOfFile
};

class ErrorInterface;
class ErrorTemplateInterface;

// Is nullptr if no error is set
using Error = std::unique_ptr<ErrorInterface>;

class ErrorInterface {
  public:
    virtual std::string Explain() const noexcept = 0;
    virtual void Append(const std::string& value) noexcept = 0;
    virtual ErrorType GetErrorType() const noexcept = 0;
    virtual ~ErrorInterface() = default; // needed for unique_ptr to delete itself

    /*TODO: Add these functions, so it will be really easy and convenient to use the error class
     * virtual inline bool operator == (const Error& rhs) const
     * virtual inline bool operator != (const Error& rhs) const
     * virtual inline bool operator == (const ErrorTemplateInterface& rhs) const
     * virtual inline bool operator != (const ErrorTemplateInterface& rhs) const
     * virtual inline bool bool() const;
     * virtual inline bool operator = (const ErrorTemplateInterface& rhs) const
     */
};


class ErrorTemplateInterface {
  public:
    virtual ErrorType GetErrorType() const noexcept = 0;
    virtual Error Generate() const noexcept = 0;
    /*TODO: Add these functions, so it will be really easy and convenient to use the error class
     * virtual inline bool operator ErrorTemplateInterface() const
     */

    /*
    virtual inline bool operator == (const Error* rhs) const {
        return rhs != nullptr &&
               operator==(*rhs);
    }

    virtual inline bool operator != (const Error* rhs) const {
        return rhs != nullptr &&
               operator!=(*rhs);
    }
     */

    virtual inline bool operator == (const Error& rhs) const {
        return rhs != nullptr &&
               GetErrorType() == rhs->GetErrorType();
    }

    virtual inline bool operator != (const Error& rhs) const {
        return !(operator==(rhs));
    }
};

static inline bool operator == (const Error& lhs, const ErrorTemplateInterface& rhs) {
    return rhs.operator == (lhs);
}

static inline bool operator != (const Error& lhs, const ErrorTemplateInterface& rhs) {
    return rhs.operator != (lhs);
}

static inline std::ostream& operator<<(std::ostream& os, const Error& err) {
    if(err) {
        os << err->Explain();
    } else {
        static std::string no_error = "No error";
        os << no_error;
    }
    return os;
}


class SimpleError: public ErrorInterface {
  private:
    std::string error_;
    ErrorType error_type_ = ErrorType::kHidraError;
  public:
    explicit SimpleError(std::string error): error_{std::move(error)} {

    }
    SimpleError(std::string error, ErrorType error_type ): error_{std::move(error)}, error_type_{error_type} {
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

class SimpleErrorTemplate : public ErrorTemplateInterface {
  protected:
    std::string error_;
    ErrorType error_type_ = ErrorType::kHidraError;
  public:
    explicit SimpleErrorTemplate(std::string error): error_{std::move(error)} {

    }
    SimpleErrorTemplate(std::string error, ErrorType error_type ): error_{std::move(error)}, error_type_{error_type} {
    }

    inline ErrorType GetErrorType() const noexcept override {
        return error_type_;
    }

    inline Error Generate() const noexcept override {
        return Error(new SimpleError{error_, error_type_});
    }
};

inline Error TextError(const std::string& error) {
    return Error{new SimpleError{error}};
}

inline Error TextErrorWithType(const std::string& error, ErrorType error_type) {
    return Error{new SimpleError{error, error_type}};
}

namespace ErrorTemplates {
auto const kMemoryAllocationError = SimpleErrorTemplate{"kMemoryAllocationError", ErrorType::kMemoryAllocationError};
auto const kEndOfFile = SimpleErrorTemplate{"End of file", ErrorType::kEndOfFile};

}

}
#endif //HIDRA2_ERROR_H
