#ifndef ASAPO_WORKER_ERROR_H
#define ASAPO_WORKER_ERROR_H

#include "common/error.h"
#include "common/io_error.h"

namespace asapo {

enum class WorkerErrorType {
    kMemoryError,
    kEmptyDatasource,
    kSourceNotFound,
    kSourceNotConnected,
    kSourceAlreadyConnected,
    kErrorReadingSource,
    kNotFound,
    kPermissionDenied,
    kNoData,
    kWrongInput,
    kAuthorizationError,
    kInternalError,
    kUnknownIOError
};


using WorkerErrorTemplate = ServiceErrorTemplate<WorkerErrorType, ErrorType::kWorkerError>;

namespace WorkerErrorTemplates {

auto const kMemoryError = WorkerErrorTemplate{
    "Memory Error", WorkerErrorType::kMemoryError
};

auto const kEmptyDatasource = WorkerErrorTemplate{
    "Empty Data Source", WorkerErrorType::kEmptyDatasource
};

auto const kSourceNotFound = WorkerErrorTemplate{
    "Source Not Found", WorkerErrorType::kSourceNotFound
};

auto const kSourceNotConnected = WorkerErrorTemplate{
    "Source Not Connected", WorkerErrorType::kSourceNotConnected
};

auto const kSourceAlreadyConnected = WorkerErrorTemplate{
    "Source Already Connected", WorkerErrorType::kSourceAlreadyConnected
};

auto const kErrorReadingSource = WorkerErrorTemplate{
    "Error Reading Source", WorkerErrorType::kErrorReadingSource
};

auto const kNotFound = WorkerErrorTemplate{
    "Uri not found", WorkerErrorType::kNotFound
};

auto const kPermissionDenied = WorkerErrorTemplate{
    "Permission Denied", WorkerErrorType::kPermissionDenied
};

auto const kNoData = WorkerErrorTemplate{
    "No Data", WorkerErrorType::kNoData
};

auto const kWrongInput = WorkerErrorTemplate{
    "Wrong Input", WorkerErrorType::kWrongInput
};

auto const kAuthorizationError = WorkerErrorTemplate{
    "Authorization Error", WorkerErrorType::kAuthorizationError
};

auto const kInternalError = WorkerErrorTemplate{
    "Internal Error", WorkerErrorType::kInternalError
};

auto const kUnknownIOError = WorkerErrorTemplate{
    "Unknown IO Error", WorkerErrorType::kUnknownIOError
};

}
}

#endif //ASAPO_WORKER_ERROR_H
