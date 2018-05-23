#ifndef ASAPO_PRODUCER_ERROR_H
#define ASAPO_PRODUCER_ERROR_H

#include "common/error.h"

namespace asapo {

enum class ProducerErrorType {
    kAlreadyConnected,
    kConnectionNotReady,
    kFileTooLarge,
    kFileIdAlreadyInUse,
    kInternalServerError,
    kCannotSendDataToReceivers,
    kRequestPoolIsFull
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

static inline std::ostream& operator<<(std::ostream& os, const ProducerErrorTemplate& err) {
    return os << err.Text();
}


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

auto const kInternalServerError = ProducerErrorTemplate {
    "Internal server error", ProducerErrorType::kInternalServerError
};

auto const kCannotSendDataToReceivers = ProducerErrorTemplate {
    "Cannot connect/send data to receivers", ProducerErrorType::kCannotSendDataToReceivers
};

auto const kRequestPoolIsFull = ProducerErrorTemplate {
    "Cannot add request to poll - hit pool size limit", ProducerErrorType::kRequestPoolIsFull
};




};
}

#endif //ASAPO_PRODUCER_ERROR_H
