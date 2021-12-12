#include "error.h"

#include "asapo/common/utils.h"

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
std::string ServiceError<ServiceErrorType>::ExplainPretty(uint8_t shift) const noexcept {
    std::string base_shift(static_cast<size_t>(shift), ' ');
    std::string shift_s(static_cast<size_t>(2), ' ');
    std::string err = base_shift + "error: " + error_name_;
    if (!error_message_.empty()) {
        err += "\n" + base_shift + shift_s + "message: " + error_message_;
    }
    if (!details_.empty()) {
        err += "\n" + base_shift + shift_s + "details: ";
        auto i = 0;
        for (const auto &kv : details_) {
            err += (i > 0 ? ", " : "") + kv.first + ":" + kv.second;
            i++;
        }
    }
    if (cause_err_ != nullptr) {
        err += "\n" + base_shift + shift_s + "caused by: ";
        err += "\n" + base_shift + shift_s + cause_err_->ExplainPretty(shift + 2);
    }
    return err;
}

template<typename ServiceErrorType>
std::string ServiceError<ServiceErrorType>::Explain() const noexcept {
    std::string err = "error: " + error_name_;
    if (!error_message_.empty()) {
        err += ", message: " + error_message_;
    }
    if (!details_.empty()) {
        err += ", details: ";
        auto i = 0;
        for (const auto &kv : details_) {
            err += (i > 0 ? ", " : "") + kv.first + ":" + kv.second;
            i++;
        }
    }
    if (cause_err_ != nullptr) {
        err +=  ", caused by: " + cause_err_->Explain() ;
    }
    return err;
}

template<typename ServiceErrorType>
ErrorInterface *ServiceError<ServiceErrorType>::AddDetails(std::string key, std::string value) noexcept {
    details_[std::move(key)] = std::move(value);
    return this;
}
template<typename ServiceErrorType>
ErrorInterface *ServiceError<ServiceErrorType>::SetCause(Error cause_err) noexcept {
    cause_err_ = std::move(cause_err);
    return this;
}

inline std::string WrapInQuotes(const std::string &origin) {
    return "\"" + origin + "\"";
}

template<typename ServiceErrorType>
std::string ServiceError<ServiceErrorType>::ExplainInJSON() const noexcept {
    std::string err = WrapInQuotes("error") + ":" + WrapInQuotes(error_name_);
    if (!error_message_.empty()) {
        err += "," + WrapInQuotes("message") + ":" + WrapInQuotes(EscapeJson(error_message_));
    }
    if (!details_.empty()) {
        err += "," + WrapInQuotes("details") + ":{";
        auto i = 0;
        for (const auto &kv : details_) {
            err += (i > 0 ? ", " : "") + WrapInQuotes(kv.first) + ":" + WrapInQuotes(EscapeJson(kv.second));
            i++;
        }
        err += "}";
    }
    if (cause_err_ != nullptr) {
        err += ","+ WrapInQuotes("cause")+":{"+cause_err_->ExplainInJSON()+"}";
    }
    return err;
}
template<typename ServiceErrorType>
const Error &ServiceError<ServiceErrorType>::GetCause() const noexcept {
    return cause_err_;
}

template<typename ServiceErrorType>
ErrorInterface *ServiceError<ServiceErrorType>::AddDetails(std::string key, uint64_t value) noexcept {
    return AddDetails(key,std::to_string(value));
}

template<typename ServiceErrorType>
Error ServiceErrorTemplate<ServiceErrorType>::Generate() const noexcept {
    return Generate("");
}

template<typename ServiceErrorType>
Error ServiceErrorTemplate<ServiceErrorType>::Generate(std::string error_message) const noexcept {
    return Error(new ServiceError<ServiceErrorType>(error_name_, std::move(error_message), error_type_));
}

template<typename ServiceErrorType>
Error ServiceErrorTemplate<ServiceErrorType>::Generate(std::string error_message, Error cause) const noexcept {
    auto err = Generate(std::move(error_message));
    err->SetCause(std::move(cause));
    return err;
}

template<typename ServiceErrorType>
Error ServiceErrorTemplate<ServiceErrorType>::Generate(Error cause) const noexcept {
    return Generate("",std::move(cause));
}

}
