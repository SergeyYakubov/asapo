#ifndef ASAPO_PRODUCER_ERROR_H
#define ASAPO_PRODUCER_ERROR_H

#include "common/error.h"

namespace asapo {

enum class ProducerErrorType {
    kInternalServerError,
    kRequestPoolIsFull,
    kLocalIOError,
    kWrongInput,
    kServerWarning,
    kTimeout
};

using ProducerErrorTemplate = ServiceErrorTemplate<ProducerErrorType, ErrorType::kProducerError>;

namespace ProducerErrorTemplates {

auto const kServerWarning = ProducerErrorTemplate {
    "server warning", ProducerErrorType::kServerWarning
};

auto const kLocalIOError = ProducerErrorTemplate {
    "local i/o error", ProducerErrorType::kLocalIOError
};

auto const kRequestPoolIsFull = ProducerErrorTemplate {
    "Cannot add request to poll - hit pool size limit", ProducerErrorType::kRequestPoolIsFull
};

auto const kWrongInput = ProducerErrorTemplate {
    "wrong input", ProducerErrorType::kWrongInput
};

auto const kInternalServerError = ProducerErrorTemplate {
    "Internal server error", ProducerErrorType::kInternalServerError
};

auto const kTimeout = ProducerErrorTemplate {
    "Timeout", ProducerErrorType::kTimeout
};


};
}

#endif //ASAPO_PRODUCER_ERROR_H

