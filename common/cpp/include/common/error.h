#ifndef ASAPO_ERROR_H
#define ASAPO_ERROR_H

#include <string>
#include <memory>
#include <utility>

namespace asapo {

enum class ErrorType {
    kUnknownError,

    kHidraError,
    kHttpError,
    kIOError,
    kReceiverError,
    kProducerError,

    kMemoryAllocationError,
    kEndOfFile,
    kTimeOut
};

class ErrorInterface;
class ErrorTemplateInterface;

// nullptr == noError
// Example check:
//  void TestError(Error* err) {
//      if(*err) {
//          [...] //An error occurred
//      }
//  }
using Error = std::unique_ptr<ErrorInterface>;

class ErrorInterface {
  public:
    virtual std::string Explain() const noexcept = 0;
    virtual void Append(const std::string& value) noexcept = 0;
    virtual ErrorType GetErrorType() const noexcept = 0;
    virtual ~ErrorInterface() = default; // needed for unique_ptr to delete itself
};


class ErrorTemplateInterface {
  public:
    virtual ErrorType GetErrorType() const noexcept = 0;
    virtual Error Generate() const noexcept = 0;
    virtual std::string Text() const noexcept = 0;

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


/*
 * IMPORTANT:
 * Never use the same ErrorType for two different errors,
 * otherwise the == operator might not work as expected!
 */
class SimpleErrorTemplate : public ErrorTemplateInterface {
  protected:
    std::string error_;
    ErrorType error_type_ = ErrorType::kHidraError;
  public:
    explicit SimpleErrorTemplate(std::string error): error_{std::move(error)} {

    }

    virtual std::string Text() const noexcept override {
        return error_;
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

static inline std::ostream& operator<<(std::ostream& os, const SimpleErrorTemplate& err) {
    return os << err.Text();
}


inline Error TextError(const std::string& error) {
    return Error{new SimpleError{error}};
}

inline Error TextErrorWithType(const std::string& error, ErrorType error_type) {
    return Error{new SimpleError{error, error_type}};
}

namespace ErrorTemplates {
auto const kMemoryAllocationError = SimpleErrorTemplate {
    "kMemoryAllocationError", ErrorType::kMemoryAllocationError
};
auto const kEndOfFile = SimpleErrorTemplate {
    "End of file", ErrorType::kEndOfFile
};

}

template <typename ServiceErrorType, ErrorType MainErrorType>
class ServiceError : public SimpleError {
 private:
  ServiceErrorType error_type_;
 public:
  ServiceError(const std::string& error, ServiceErrorType error_type) : SimpleError(error, MainErrorType) {
      error_type_ = error_type;
  }
  ServiceErrorType GetServiceErrorType() const noexcept {
      return error_type_;
  }
};

template <typename ServiceErrorType, ErrorType MainErrorType>
class ServiceErrorTemplate : public SimpleErrorTemplate {
 protected:
  ServiceErrorType error_type_;
 public:
  ServiceErrorTemplate(const std::string& error, ServiceErrorType error_type) : SimpleErrorTemplate(error,
                                                                                                    MainErrorType) {
      error_type_ = error_type;
  }

  inline ServiceErrorType GetServiceErrorType() const noexcept {
      return error_type_;
  }

  inline Error Generate() const noexcept override {
      auto err = new ServiceError<ServiceErrorType, MainErrorType>(error_, error_type_);
      return Error(err);
  }

  inline Error Generate(const std::string& prefix) const noexcept {
      auto err = new ServiceError<ServiceErrorType, MainErrorType>(prefix + " :" + error_, error_type_);
      return Error(err);
  }

  inline bool operator==(const Error& rhs) const override {
      return SimpleErrorTemplate::operator==(rhs)
          && GetServiceErrorType() == ((ServiceError<ServiceErrorType, MainErrorType>*) rhs.get())->GetServiceErrorType();
  }
};

}
#endif //ASAPO_ERROR_H
