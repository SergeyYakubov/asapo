#include "http_client.h"

namespace hidra2 {

WorkerErrorCode HttpCodeToWorkerError(const HttpCode& code) {
    switch (code) {
    case HttpCode::OK:
        return WorkerErrorCode::kOK;
    case HttpCode::BadRequest:
        return WorkerErrorCode::kWrongInput;
    case HttpCode::InternalServerError:
        return WorkerErrorCode::kErrorReadingSource;
    case HttpCode::NoContent:
        return WorkerErrorCode::kNoData;
    case HttpCode::NotFound:
        return WorkerErrorCode::kSourceNotFound;
    default:
        return WorkerErrorCode::kErrorReadingSource;
    }

}

}
