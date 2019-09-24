#ifndef ASAPO_PRODUCER_ERROR_H
#define ASAPO_PRODUCER_ERROR_H

#include "common/error.h"

namespace asapo {

enum class ProducerErrorType {
    kFileTooLarge,
    kFileNameTooLong,
    kEmptyFileName,
    kNoData,
    kZeroDataSize,
    kBeamtimeIdTooLong,
    kBeamtimeAlreadySet,
    kFileIdAlreadyInUse,
    kErrorInMetadata,
    kErrorSubsetSize,
    kAuthorizationFailed,
    kInternalServerError,
    kCannotSendDataToReceivers,
    kRequestPoolIsFull,
    kWrongIngestMode
};

using ProducerErrorTemplate = ServiceErrorTemplate<ProducerErrorType, ErrorType::kProducerError>;

namespace ProducerErrorTemplates {


auto const kWrongIngestMode = ProducerErrorTemplate {
    "wrong ingest mode", ProducerErrorType::kWrongIngestMode
};

auto const kNoData = ProducerErrorTemplate {
    "no data", ProducerErrorType::kNoData
};


auto const kZeroDataSize = ProducerErrorTemplate {
    "zero data size", ProducerErrorType::kZeroDataSize
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

auto const kEmptyFileName = ProducerErrorTemplate {
    "empty filename", ProducerErrorType::kEmptyFileName
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

