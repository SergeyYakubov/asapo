#ifndef ASAPO_RECEIVER_DATA_SERVER_ERROR_H
#define ASAPO_RECEIVER_DATA_SERVER_ERROR_H

#include "common/error.h"

namespace asapo {

enum class ReceiverDataServerErrorType {
    kMemoryPool,
    kWrongRequest
};

using ReceiverDataServerErrorTemplate = ServiceErrorTemplate<ReceiverDataServerErrorType, ErrorType::kReceiverError>;

namespace ReceiverDataServerErrorTemplates {
auto const kMemoryPool = ReceiverDataServerErrorTemplate {
    "memory error", ReceiverDataServerErrorType::kMemoryPool
};

auto const kWrongRequest = ReceiverDataServerErrorTemplate {
    "wrong request", ReceiverDataServerErrorType::kWrongRequest
};


};
}

#endif //ASAPO_RECEIVER_DATA_SERVER_ERROR_H