#ifndef ASAPO_WORKER_ERROR_H
#define ASAPO_WORKER_ERROR_H

#include "common/error.h"
#include "common/io_error.h"

namespace asapo {

enum class WorkerErrorType {
    kNoData,
    kEndOfStream,
    kBrokerServersNotFound,
    kBrokerServerError,
    kIOError,
    kWrongInput
};

using WorkerErrorTemplate = ServiceErrorTemplate<WorkerErrorType, ErrorType::kWorkerError>;


class WorkerErrorData : public CustomErrorData {
  public:
    uint64_t id;
    uint64_t id_max;
};


namespace WorkerErrorTemplates {

auto const kIOError = WorkerErrorTemplate{
    "i/o error", WorkerErrorType::kIOError
};

auto const kEndOfStream = WorkerErrorTemplate{
    "no data - end of stream", WorkerErrorType::kEndOfStream
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

