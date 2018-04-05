#ifndef HIDRA2_PRODUCER_ERROR_H
#define HIDRA2_PRODUCER_ERROR_H

#include "common/error.h"

namespace hidra2 {

enum class ProducerErrorType {
    kAlreadyConnected,
    kConnectionNotReady,
    kFileTooLarge,
    kFileIdAlreadyInUse,
    kUnknownServerError
};

//TODO Make a marco to create error class and error template class
class ProducerError : public SimpleError {
  private:
    ProducerErrorType receiver_error_type_;
  public:
    ProducerError(const std::string& error, ProducerErrorType receiver_error_type) : SimpleError(error,
                ErrorType::kProducerError) {
        receiver_error_type_ = receiver_error_type;
    }

    ProducerErrorType GetProducerErrorType() const noexcept {
        return receiver_error_type_;
    }
};

class ProducerErrorTemplate : public SimpleErrorTemplate {
  protected:
    ProducerErrorType receiver_error_type_;
  public:
    ProducerErrorTemplate(const std::string& error, ProducerErrorType receiver_error_type) : SimpleErrorTemplate(error,
                ErrorType::kProducerError) {
        receiver_error_type_ = receiver_error_type;
    }

    inline ProducerErrorType GetProducerErrorType() const noexcept {
        return receiver_error_type_;
    }

    inline Error Generate() const noexcept override {
        return Error(new ProducerError(error_, receiver_error_type_));
    }

    inline bool operator==(const Error& rhs) const override {
        return SimpleErrorTemplate::operator==(rhs)
               && GetProducerErrorType() == ((ProducerError*) rhs.get())->GetProducerErrorType();
    }
};

namespace ProducerErrorTemplates {
auto const kAlreadyConnected = ProducerErrorTemplate {
    "Already connected", ProducerErrorType::kAlreadyConnected
};
auto const kConnectionNotReady = ProducerErrorTemplate {
    "Connection not ready", ProducerErrorType::kConnectionNotReady
};

auto const kFileTooLarge = ProducerErrorTemplate {
    "File too large", ProducerErrorType::kFileTooLarge
};

auto const kFileIdAlreadyInUse = ProducerErrorTemplate {
    "File already in use", ProducerErrorType::kFileIdAlreadyInUse
};

auto const kUnknownServerError = ProducerErrorTemplate {
    "Unknown server error", ProducerErrorType::kUnknownServerError
};


};
}

#endif //HIDRA2_PRODUCER_ERROR_H
