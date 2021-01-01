#ifndef ASAPO_HTTP_ERROR_H
#define ASAPO_HTTP_ERROR_H

#include "asapo/common/error.h"
#include "http_client.h"

namespace asapo {

enum class HttpErrorType {
    kTransferError,
    kConnectionError
};

using HttpErrorTemplate = ServiceErrorTemplate<HttpErrorType, ErrorType::kHttpError>;

namespace HttpErrorTemplates {

auto const kTransferError = HttpErrorTemplate {
    "possible transfer error", HttpErrorType::kTransferError
};

auto const kConnectionError = HttpErrorTemplate {
    "connection error", HttpErrorType::kConnectionError
};

}

}

#endif //ASAPO_HTTP_ERROR_H
