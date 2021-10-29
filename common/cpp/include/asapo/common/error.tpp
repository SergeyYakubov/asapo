
#include "error.h"

namespace asapo {

template<typename ServiceErrorType>
ServiceError<ServiceErrorType>::ServiceError(std::string error_name,
                                             std::string error_message,
                                             ServiceErrorType error_type) : error_type_{
    error_type}, error_name_{std::move(error_name)}, error_message_{std::move(error_message)} {
}

template<typename ServiceErrorType>
ServiceErrorType ServiceError<ServiceErrorType>::GetServiceErrorType() const noexcept {
    return error_type_;
}

template<typename ServiceErrorType>
CustomErrorData *ServiceError<ServiceErrorType>::GetCustomData() noexcept {
    if (custom_data_) {
        return custom_data_.get();
    } else {
        return nullptr;
    }
}

template<typename ServiceErrorType>
void ServiceError<ServiceErrorType>::SetCustomData(std::unique_ptr<CustomErrorData> data) noexcept {
    custom_data_ = std::move(data);
}

template<typename ServiceErrorType>
void ServiceError<ServiceErrorType>::Append(const std::string &value) noexcept {
    error_message_ += ": " + value;
}

template<typename ServiceErrorType>
void ServiceError<ServiceErrorType>::Prepend(const std::string &value) noexcept {
    error_message_ = value + ": " + error_message_;

}

template<typename ServiceErrorType>
std::string ServiceError<ServiceErrorType>::Explain() const noexcept {
    return error_name_ + ": " + error_message_;
}

template<typename ServiceErrorType>
ErrorInterface *ServiceError<ServiceErrorType>::AddContext(std::string context) noexcept {
    context_ = std::move(context);
    return this;
}
template<typename ServiceErrorType>
ErrorInterface *ServiceError<ServiceErrorType>::AddCause(Error cause_err) noexcept {
    cause_err_ = std::move(cause_err);
    return this;
}

template<typename ServiceErrorType>
std::string ServiceError<ServiceErrorType>::ExplainInJSON() const noexcept {
    return std::string();
}

template<typename ServiceErrorType>
Error ServiceErrorTemplate<ServiceErrorType>::Generate() const noexcept {
    auto err = new ServiceError<ServiceErrorType>(error_name_, "", error_type_);
    return Error(err);
}
template<typename ServiceErrorType>
Error ServiceErrorTemplate<ServiceErrorType>::Generate(std::string error_message) const noexcept {
    return Error(new ServiceError<ServiceErrorType>(error_name_, std::move(error_message), error_type_));
}

}
