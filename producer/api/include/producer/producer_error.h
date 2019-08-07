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
    kErrorInMetadata,
    kErrorSubsetSize,
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

auto const kErrorSubsetSize = ProducerErrorTemplate {
    "Error in subset size", ProducerErrorType::kErrorSubsetSize
};


auto const kFileTooLarge = ProducerErrorTemplate {
    "File too large", ProducerErrorType::kFileTooLarge
};

auto const kFileNameTooLong = ProducerErrorTemplate {
    "filename too long", ProducerErrorType::kFileNameTooLong
};

auto const kCredentialsTooLong = ProducerErrorTemplate {
    "beamtime id too long", ProducerErrorType::kBeamtimeIdTooLong
};


auto const kCredentialsAlreadySet = ProducerErrorTemplate {
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

auto const kErrorInMetadata = ProducerErrorTemplate {
    "error in metadata", ProducerErrorType::kErrorInMetadata
};





};
}

#endif //ASAPO_PRODUCER_ERROR_H
