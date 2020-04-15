#ifndef ASAPO_RECEIVER_ERROR_H
#define ASAPO_RECEIVER_ERROR_H

#include "common/error.h"

namespace asapo {

enum class ReceiverErrorType {
    kInvalidOpCode,
    kBadRequest,
    kAuthorizationFailure,
    kInternalServerError,
    kReAuthorizationFailure,
    kWarningDuplicatedRequest
};

using ReceiverErrorTemplate = ServiceErrorTemplate<ReceiverErrorType, ErrorType::kReceiverError>;


namespace ReceiverErrorTemplates {

auto const kWarningDuplicatedRequest = ReceiverErrorTemplate {
    "Duplicated request, possible due to retry", ReceiverErrorType::kWarningDuplicatedRequest
};


auto const kInvalidOpCode = ReceiverErrorTemplate {
    "Invalid Opcode", ReceiverErrorType::kInvalidOpCode
};

auto const kInternalServerError = ReceiverErrorTemplate {
    "server error", ReceiverErrorType::kInternalServerError
};


auto const kBadRequest = ReceiverErrorTemplate {
    "Bad request", ReceiverErrorType::kBadRequest
};

auto const kAuthorizationFailure = ReceiverErrorTemplate {
    "authorization failure", ReceiverErrorType::kAuthorizationFailure
};

auto const kReAuthorizationFailure = ReceiverErrorTemplate {
    "reauthorization for auto beamtime failed", ReceiverErrorType::kReAuthorizationFailure
};

};
}

#endif //ASAPO_RECEIVER_ERROR_H
