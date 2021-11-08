#ifndef ASAPO_ERROR_H
#define ASAPO_ERROR_H

#include <string>
#include <memory>
#include <utility>
#include <ostream>
#include <map>

namespace asapo {

class ErrorInterface;
class ErrorTemplateInterface;
class CustomErrorData;


using Error = std::unique_ptr<ErrorInterface>;

class ErrorInterface {
  public:
    virtual std::string Explain() const noexcept = 0;
    virtual std::string ExplainPretty(uint8_t shift = 0) const noexcept = 0;
    virtual std::string ExplainInJSON() const noexcept = 0;
    virtual ErrorInterface* AddContext(std::string key, std::string value) noexcept = 0;
    virtual ErrorInterface* SetCause(Error cause_err) noexcept = 0;
    virtual const Error& GetCause() const noexcept = 0;
    virtual CustomErrorData* GetCustomData() noexcept = 0;
    virtual void SetCustomData(std::unique_ptr<CustomErrorData> data) noexcept = 0;
    virtual ~ErrorInterface() = default; // needed for unique_ptr to delete itself
};

static inline std::ostream& operator<<(std::ostream& os, const Error& err) {
    if (err) {
        os << err->Explain();
    } else {
        static std::string no_error = "No error";
        os << no_error;
    }
    return os;
}

class CustomErrorData {
  public:
    virtual ~CustomErrorData() = default;
};

template<typename ServiceErrorType>
class ServiceError : public ErrorInterface {
  private:
    ServiceErrorType error_type_;
    std::string error_name_;
    std::string error_message_;
    std::map<std::string, std::string> context_;
    Error cause_err_;
    std::unique_ptr<CustomErrorData> custom_data_;
  public:
    ServiceError(std::string error_name, std::string error_message, ServiceErrorType error_type);
    ServiceErrorType GetServiceErrorType() const noexcept;
    CustomErrorData* GetCustomData() noexcept override;
    void SetCustomData(std::unique_ptr<CustomErrorData> data) noexcept override;
    ErrorInterface* AddContext(std::string key, std::string value) noexcept override;
    ErrorInterface* SetCause(Error cause_err) noexcept override;
    const Error& GetCause() const noexcept override;
    std::string Explain() const noexcept override;
    virtual std::string ExplainPretty(uint8_t shift) const noexcept override;
    std::string ExplainInJSON() const noexcept override;
};

class ErrorTemplateInterface {
  public:
    virtual Error Generate() const noexcept = 0;
    virtual Error Generate(std::string error_message) const noexcept = 0;
    virtual Error Generate(std::string error_message, Error cause) const noexcept = 0;
    virtual Error Generate(Error cause) const noexcept = 0;
    virtual bool operator==(const Error& rhs) const = 0;
    virtual bool operator!=(const Error& rhs) const = 0;

};

static inline bool operator==(const Error& lhs, const ErrorTemplateInterface& rhs) {
    return rhs == lhs;
}

static inline bool operator!=(const Error& lhs, const ErrorTemplateInterface& rhs) {
    return rhs != lhs;
}

template<typename ServiceErrorType>
class ServiceErrorTemplate : public ErrorTemplateInterface {
  private:
    std::string error_name_;
    ServiceErrorType error_type_;
  public:
    ServiceErrorTemplate(const std::string& error_name, ServiceErrorType error_type) {
        error_name_ = error_name;
        error_type_ = error_type;
    }

    const std::string& Text() const noexcept {
        return error_name_;
    }

    ServiceErrorType GetServiceErrorType() const noexcept {
        return error_type_;
    }

    Error Generate() const noexcept override;

    Error Generate(std::string error_message) const noexcept override;
    Error Generate(std::string error_message, Error cause) const noexcept override;
    Error Generate(Error cause) const noexcept override;
    inline bool operator==(const Error& rhs) const override {
        return rhs != nullptr
               && GetServiceErrorType() == ((ServiceError<ServiceErrorType>*) rhs.get())->GetServiceErrorType();
    }

    inline bool operator!=(const Error& rhs) const override {
        return rhs != nullptr
               && GetServiceErrorType() != ((ServiceError<ServiceErrorType>*) rhs.get())->GetServiceErrorType();
    }

};

template<typename ServiceErrorType>
static inline bool operator==(const Error& lhs, const ServiceErrorTemplate<ServiceErrorType>& rhs) {
    return rhs == lhs;
}

template<typename ServiceErrorType>
static inline bool operator!=(const Error& lhs, const ServiceErrorTemplate<ServiceErrorType>& rhs) {
    return rhs != lhs;
}

namespace GeneralErrorTemplates {

enum class GeneralErrorType {
    kMemoryAllocationError,
    kEndOfFile,
    kSimpleError,
};

using GeneralError = ServiceError<GeneralErrorType>;
using GeneralErrorTemplate = ServiceErrorTemplate<GeneralErrorType>;

auto const kMemoryAllocationError = GeneralErrorTemplate {
    "memory allocation", GeneralErrorType::kMemoryAllocationError
};

auto const kEndOfFile = GeneralErrorTemplate {
    "end of file", GeneralErrorType::kEndOfFile
};

auto const kSimpleError = GeneralErrorTemplate {
    "unnamed error", GeneralErrorType::kSimpleError
};

}

}

#include "error.tpp"


#endif //ASAPO_ERROR_H
