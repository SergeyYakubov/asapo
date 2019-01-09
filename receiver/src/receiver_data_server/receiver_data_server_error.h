#ifndef ASAPO_RECEIVER_ERROR_H
#define ASAPO_RECEIVER_ERROR_H

#include "common/error.h"

namespace asapo {

enum class ReceiverDataServerErrorType {
    kMemoryPool,
};

using ReceiverDataServerErrorTemplate = ServiceErrorTemplate<ReceiverDataServerErrorType, ErrorType::kReceiverError>;

namespace ReceiverErrorTemplates {
auto const kMemoryPool = ReceiverDataServerErrorTemplate {
    "memory error", ReceiverDataServerErrorType::kMemoryPool
};

};
}

#endif //ASAPO_RECEIVER_ERROR_H
