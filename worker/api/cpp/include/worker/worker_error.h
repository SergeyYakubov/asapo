#ifndef ASAPO_WORKER_ERROR_H
#define ASAPO_WORKER_ERROR_H

#include "common/error.h"
#include "common/io_error.h"

namespace asapo {

enum class WorkerErrorType {
    kNoData,
    kBrokerServersNotFound,
    kBrokerServerError,
    kIOError,
    kWrongInput
};

using WorkerErrorTemplate = ServiceErrorTemplate<WorkerErrorType, ErrorType::kWorkerError>;

namespace WorkerErrorTemplates {

auto const kIOError = WorkerErrorTemplate{
    "i/o error", WorkerErrorType::kIOError
};


auto const kNoData = WorkerErrorTemplate{
    "no data", WorkerErrorType::kNoData
};

auto const kWrongInput = WorkerErrorTemplate{
    "wrong input", WorkerErrorType::kWrongInput
};

auto const kBrokerServerError = WorkerErrorTemplate{
    "error from broker server", WorkerErrorType::kBrokerServerError
};

auto const kBrokerServersNotFound = WorkerErrorTemplate{
    "cannot find brokers", WorkerErrorType::kBrokerServersNotFound
};



}
}

#endif //ASAPO_WORKER_ERROR_H
