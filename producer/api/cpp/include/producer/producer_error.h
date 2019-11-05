#ifndef ASAPO_PRODUCER_ERROR_H
#define ASAPO_PRODUCER_ERROR_H

#include "common/error.h"

namespace asapo {

enum class ProducerErrorType {
    kInternalServerError,
    kRequestPoolIsFull,
    kLocalIOError,
    kWrongInput
};

using ProducerErrorTemplate = ServiceErrorTemplate<ProducerErrorType, ErrorType::kProducerError>;

namespace ProducerErrorTemplates {

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

};
}

#endif //ASAPO_PRODUCER_ERROR_H

