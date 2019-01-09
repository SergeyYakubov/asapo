#ifndef ASAPO_PRODUCER_ERROR_H
#define ASAPO_PRODUCER_ERROR_H

#include "common/error.h"

namespace asapo {

enum class ProducerErrorType {
    kAlreadyConnected,
    kConnectionNotReady,
    kFileTooLarge,
    kFileNameTooLong,
    kBeamtimeIdTooLong,
    kBeamtimeAlreadySet,
    kFileIdAlreadyInUse,
    kAuthorizationFailed,
    kInternalServerError,
    kCannotSendDataToReceivers,
    kRequestPoolIsFull
};

using ProducerErrorTemplate = ServiceErrorTemplate<ProducerErrorType, ErrorType::kProducerError>;

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

auto const kFileNameTooLong = ProducerErrorTemplate {
    "filename too long", ProducerErrorType::kFileNameTooLong
};

auto const kBeamtimeIdTooLong = ProducerErrorTemplate {
    "beamtime id too long", ProducerErrorType::kBeamtimeIdTooLong
};


auto const kBeamtimeAlreadySet = ProducerErrorTemplate {
    "beamtime id already set", ProducerErrorType::kBeamtimeAlreadySet
};


auto const kFileIdAlreadyInUse = ProducerErrorTemplate {
    "File already in use", ProducerErrorType::kFileIdAlreadyInUse
};

auto const kAuthorizationFailed = ProducerErrorTemplate {
    "Authorization failed:", ProducerErrorType::kAuthorizationFailed
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
