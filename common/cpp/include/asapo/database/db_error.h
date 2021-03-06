#ifndef ASAPO_SYSTEM__DB_ERROR_H
#define ASAPO_SYSTEM__DB_ERROR_H

#include "asapo/common/error.h"

namespace asapo {


enum class DBErrorType {
    kJsonParseError,
    kUnknownError,
    kConnectionError,
    kNotConnected,
    kInsertError,
    kDuplicateID,
    kAlreadyConnected,
    kBadAddress,
    kMemoryError,
    kNoRecord,
    kWrongInput
};

using DBError = ServiceError<DBErrorType>;
using DBErrorTemplate = ServiceErrorTemplate<DBErrorType>;

namespace DBErrorTemplates {

auto const kNoRecord = DBErrorTemplate {
    "No record", DBErrorType::kNoRecord
};

auto const kWrongInput = DBErrorTemplate {
    "Wrong input", DBErrorType::kWrongInput
};


auto const kNotConnected = DBErrorTemplate {
    "Not connected", DBErrorType::kNotConnected
};


auto const kDBError = DBErrorTemplate {
    "Unknown Error", DBErrorType::kUnknownError
};

auto const kConnectionError = DBErrorTemplate {
    "Connection error", DBErrorType::kConnectionError
};

auto const kInsertError = DBErrorTemplate {
    "Insert error", DBErrorType::kInsertError
};

auto const kJsonParseError = DBErrorTemplate {
    "JSON parse error", DBErrorType::kJsonParseError
};

auto const kDuplicateID = DBErrorTemplate {
    "Duplicate ID", DBErrorType::kDuplicateID
};

auto const kAlreadyConnected = DBErrorTemplate {
    "Not connected", DBErrorType::kAlreadyConnected
};

auto const kBadAddress = DBErrorTemplate {
    "Bad address", DBErrorType::kBadAddress
};

auto const kMemoryError = DBErrorTemplate {
    "Memory error", DBErrorType::kMemoryError
};


}

}

#endif //ASAPO_SYSTEM__DB_ERROR_H
