#ifndef ASAPO_CONSUMER_ERROR_H
#define ASAPO_CONSUMER_ERROR_H

#include "common/error.h"
#include "common/io_error.h"

namespace asapo {

enum class ConsumerErrorType {
    kNoData,
    kEndOfStream,
    kStreamFinished,
    kUnavailableService,
    kInterruptedTransaction,
    kLocalIOError,
    kWrongInput
};

using ConsumerErrorTemplate = ServiceErrorTemplate<ConsumerErrorType, ErrorType::kConsumerError>;


class ConsumerErrorData : public CustomErrorData {
  public:
    uint64_t id;
    uint64_t id_max;
    std::string next_substream;
};


namespace ConsumerErrorTemplates {

auto const kLocalIOError = ConsumerErrorTemplate{
    "local i/o error", ConsumerErrorType::kLocalIOError
};

auto const kStreamFinished = ConsumerErrorTemplate{
    "stream finished", ConsumerErrorType::kStreamFinished
};

auto const kEndOfStream = ConsumerErrorTemplate{
    "no data - end of stream", ConsumerErrorType::kEndOfStream
};

auto const kNoData = ConsumerErrorTemplate{
    "no data", ConsumerErrorType::kNoData
};

auto const kWrongInput = ConsumerErrorTemplate{
    "wrong input", ConsumerErrorType::kWrongInput
};

auto const kInterruptedTransaction = ConsumerErrorTemplate{
    "error from broker server", ConsumerErrorType::kInterruptedTransaction
};

auto const kUnavailableService = ConsumerErrorTemplate{
    "service unavailable", ConsumerErrorType::kUnavailableService
};



}
}

#endif //ASAPO_CONSUMER_ERROR_H

