#ifndef ASAPO_RECEIVER_ERROR_H
#define ASAPO_RECEIVER_ERROR_H

#include "common/error.h"

namespace asapo {

enum class ReceiverErrorType {
    kInvalidOpCode,
    kBadRequest,
    kReject,
    kAuthorizationFailure,
    kCannotConnectToDatabase
};

using ReceiverErrorTemplate = ServiceErrorTemplate<ReceiverErrorType, ErrorType::kReceiverError>;


namespace ReceiverErrorTemplates {
auto const kInvalidOpCode = ReceiverErrorTemplate{
    "Invalid Opcode", ReceiverErrorType::kInvalidOpCode
};

auto const kReject = ReceiverErrorTemplate{
    "request rejected", ReceiverErrorType::kReject
};


auto const kCannotConnectToDatabase = ReceiverErrorTemplate{
    "cannot connect to database", ReceiverErrorType::kCannotConnectToDatabase
};


auto const kBadRequest = ReceiverErrorTemplate{
    "Bad request", ReceiverErrorType::kBadRequest
};

auto const kAuthorizationFailure = ReceiverErrorTemplate{
    "authorization failure", ReceiverErrorType::kAuthorizationFailure
};

};
}

#endif //ASAPO_RECEIVER_ERROR_H
