#include "http_client.h"
#include "http_error.h"
namespace hidra2 {

Error HttpCodeToWorkerError(const HttpCode& code) {
    const char* message;
    switch (code) {
    case HttpCode::OK:
        return nullptr;
    case HttpCode::BadRequest:
        message = WorkerErrorMessage::kWrongInput;
        break;
    case HttpCode::InternalServerError:
        message = WorkerErrorMessage::kErrorReadingSource;
        break;
    case HttpCode::NoContent:
        message = WorkerErrorMessage::kNoData;
        return TextErrorWithType(message, ErrorType::kEOF);
    case HttpCode::NotFound:
        message = WorkerErrorMessage::kSourceNotFound;
        break;
    default:
        message = WorkerErrorMessage::kErrorReadingSource;
        break;
    }
    return Error{new HttpError(message, code)};
}

}
