#ifndef ASAPO_CONSUMER_ERROR_H
#define ASAPO_CONSUMER_ERROR_H

#include "asapo/common/error.h"
#include "asapo/common/io_error.h"

namespace asapo {

enum class ConsumerErrorType {
    kNoData,
    kEndOfStream,
    kStreamFinished,
    kUnavailableService,
    kInterruptedTransaction,
    kLocalIOError,
    kWrongInput,
    kPartialData,
    kUnsupportedClient
};

using ConsumerErrorTemplate = ServiceErrorTemplate<ConsumerErrorType, ErrorType::kConsumerError>;


class PartialErrorData : public CustomErrorData {
  public:
    uint64_t id;
    uint64_t expected_size;
};

class ConsumerErrorData : public CustomErrorData {
  public:
    uint64_t id;
    uint64_t id_max;
    std::string next_stream;
};


namespace ConsumerErrorTemplates {


auto const kPartialData = ConsumerErrorTemplate {
    "partial data", ConsumerErrorType::kPartialData
};



auto const kLocalIOError = ConsumerErrorTemplate {
    "local i/o error", ConsumerErrorType::kLocalIOError
};

auto const kStreamFinished = ConsumerErrorTemplate {
    "stream finished", ConsumerErrorType::kStreamFinished
};

auto const kEndOfStream = ConsumerErrorTemplate {
    "no data - end of stream", ConsumerErrorType::kEndOfStream
};

auto const kNoData = ConsumerErrorTemplate {
    "no data", ConsumerErrorType::kNoData
};

auto const kWrongInput = ConsumerErrorTemplate {
    "wrong input", ConsumerErrorType::kWrongInput
};

auto const kUnsupportedClient = ConsumerErrorTemplate {
    "unsupported client version", ConsumerErrorType::kUnsupportedClient
};


auto const kInterruptedTransaction = ConsumerErrorTemplate {
    "server error", ConsumerErrorType::kInterruptedTransaction
};

auto const kUnavailableService = ConsumerErrorTemplate {
    "service unavailable", ConsumerErrorType::kUnavailableService
};



}
}

#endif //ASAPO_CONSUMER_ERROR_H

