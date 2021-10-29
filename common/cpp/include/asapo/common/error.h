#ifndef ASAPO_ERROR_H
#define ASAPO_ERROR_H

#include <string>
#include <memory>
#include <utility>
#include <ostream>
namespace asapo {

class ErrorInterface;
class ErrorTemplateInterface;
class CustomErrorData;

using Error = std::unique_ptr<ErrorInterface>;

class ErrorInterface {
  public:
    virtual std::string Explain() const noexcept = 0;
    virtual void Append(const std::string& value) noexcept = 0;
    virtual void Prepend(const std::string& value) noexcept = 0;
    virtual CustomErrorData* GetCustomData() = 0;
    virtual void SetCustomData(std::unique_ptr<CustomErrorData> data) = 0;
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
    std::string error_;
    std::unique_ptr<CustomErrorData> custom_data_;
  public:
    ServiceError(const std::string& error, ServiceErrorType error_type) : error_type_{error_type},
        error_{std::move(error)} {
    }
    ServiceErrorType GetServiceErrorType() const noexcept {
        return error_type_;
    }
    CustomErrorData* GetCustomData() override {
        if (custom_data_) {
            return custom_data_.get();
        } else {
            return nullptr;
        }
    };

    void SetCustomData(std::unique_ptr<CustomErrorData> data) override {
        custom_data_ = std::move(data);
    }

    void Append(const std::string& value) noexcept override {
        error_ += ": " + value;
    }

    void Prepend(const std::string& value) noexcept override {
        error_ = value + ": " + error_;
    }

    std::string Explain() const noexcept override {
        return error_;
    }
};

class ErrorTemplateInterface {
  public:
    virtual Error Generate() const noexcept = 0;
    virtual Error Generate(const std::string& suffix) const noexcept = 0;

    virtual bool operator==(const Error& rhs) const = 0;
    virtual bool operator!=(const Error& rhs) const = 0;

};

static inline bool operator==(const Error& lhs, const ErrorTemplateInterface& rhs) {
    return rhs.operator == (lhs);
}

static inline bool operator!=(const Error& lhs, const ErrorTemplateInterface& rhs) {
    return rhs.operator != (lhs);
}

template<typename ServiceErrorType>
class ServiceErrorTemplate : public ErrorTemplateInterface {
  private:
    std::string error_;
    ServiceErrorType error_type_;
  public:
    ServiceErrorTemplate(const std::string& error, ServiceErrorType error_type) {
        error_ = error;
        error_type_ = error_type;
    }

    const std::string& Text() const noexcept {
        return error_;
    }

    ServiceErrorType GetServiceErrorType() const noexcept {
        return error_type_;
    }

    Error Generate() const noexcept override{
        auto err = new ServiceError<ServiceErrorType>(error_, error_type_);
        return Error(err);
    }

    Error Generate(const std::string& suffix) const noexcept override {
        return Error(new ServiceError<ServiceErrorType>(error_ + (error_.empty() ? "" : ": ") + suffix, error_type_));
    }

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
    return rhs.operator == (lhs);
}

template<typename ServiceErrorType>
static inline bool operator!=(const Error& lhs, const ServiceErrorTemplate<ServiceErrorType>& rhs) {
    return rhs.operator != (lhs);
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
    "kMemoryAllocationError", GeneralErrorType::kMemoryAllocationError
};

auto const kEndOfFile = GeneralErrorTemplate {
    "End of file", GeneralErrorType::kEndOfFile
};

auto const kSimpleError = GeneralErrorTemplate {
    "", GeneralErrorType::kSimpleError
};

}

}
#endif //ASAPO_ERROR_H
