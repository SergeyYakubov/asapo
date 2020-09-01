#ifndef ASAPO_RDS_RESPONSE_ERROR_H
#define ASAPO_RDS_RESPONSE_ERROR_H

#include <common/networking.h>

namespace asapo {

using RdsResponseError = ServiceError<NetworkErrorCode, ErrorType::kFabricError>;
using RdsResponseErrorTemplate = ServiceErrorTemplate<NetworkErrorCode, ErrorType::kFabricError>;

namespace RdsResponseErrorTemplates {
auto const kNetErrorReauthorize = RdsResponseErrorTemplate {
    "RDS response Reauthorize", NetworkErrorCode::kNetErrorReauthorize
};
auto const kNetErrorWarning = RdsResponseErrorTemplate {
    "RDS response Warning", NetworkErrorCode::kNetErrorWarning
};
auto const kNetErrorWrongRequest = RdsResponseErrorTemplate {
    "RDS response WrongRequest", NetworkErrorCode::kNetErrorWrongRequest
};
auto const kNetErrorNoData = RdsResponseErrorTemplate {
    "RDS response NoData", NetworkErrorCode::kNetErrorNoData
};
auto const kNetAuthorizationError = RdsResponseErrorTemplate {
    "RDS response AuthorizationError", NetworkErrorCode::kNetAuthorizationError
};
auto const kNetErrorInternalServerError = RdsResponseErrorTemplate {
    "RDS response InternalServerError", NetworkErrorCode::kNetErrorInternalServerError
};
}

inline Error ConvertRdsResponseToError(NetworkErrorCode error_code) {
    switch (error_code) {
    case kNetErrorNoError:
        return nullptr;
    case kNetErrorReauthorize:
        return RdsResponseErrorTemplates::kNetErrorReauthorize.Generate();
    case kNetErrorWarning:
        return RdsResponseErrorTemplates::kNetErrorWarning.Generate();
    case kNetErrorWrongRequest:
        return RdsResponseErrorTemplates::kNetErrorWrongRequest.Generate();
    case kNetErrorNoData:
        return RdsResponseErrorTemplates::kNetErrorNoData.Generate();
    case kNetAuthorizationError:
        return RdsResponseErrorTemplates::kNetAuthorizationError.Generate();
    case kNetErrorInternalServerError:
        return RdsResponseErrorTemplates::kNetErrorInternalServerError.Generate();
    default:
        return TextError("Unknown RDS response code " + std::to_string(error_code));
    }
}
}

#endif //ASAPO_RDS_RESPONSE_ERROR_H
