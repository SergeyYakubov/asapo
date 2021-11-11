#ifndef ASAPO_RECEIVER_DATA_SERVER_ERROR_H
#define ASAPO_RECEIVER_DATA_SERVER_ERROR_H

#include "asapo/common/error.h"

namespace asapo {

enum class ReceiverDataServerErrorType {
    kMemoryPool,
    kWrongRequest
};

using ReceiverDataServerErrorTemplate = ServiceErrorTemplate<ReceiverDataServerErrorType>;

namespace ReceiverDataServerErrorTemplates {
auto const kMemoryPool = ReceiverDataServerErrorTemplate {
    "memory error", ReceiverDataServerErrorType::kMemoryPool
};

auto const kWrongRequest = ReceiverDataServerErrorTemplate {
    "wrong request", ReceiverDataServerErrorType::kWrongRequest
};


}
}

#endif //ASAPO_RECEIVER_DATA_SERVER_ERROR_H
