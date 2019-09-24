#ifndef ASAPO_CONSUMER_ERROR_H
#define ASAPO_CONSUMER_ERROR_H

#include "common/error.h"
#include "common/io_error.h"

namespace asapo {

enum class ConsumerErrorType {
    kNoData,
    kEndOfStream,
    kBrokerServersNotFound,
    kBrokerServerError,
    kIOError,
    kWrongInput
};

using ConsumerErrorTemplate = ServiceErrorTemplate<ConsumerErrorType, ErrorType::kConsumerError>;


class ConsumerErrorData : public CustomErrorData {
  public:
    uint64_t id;
    uint64_t id_max;
};


namespace ConsumerErrorTemplates {

auto const kIOError = ConsumerErrorTemplate{
    "i/o error", ConsumerErrorType::kIOError
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

auto const kBrokerServerError = ConsumerErrorTemplate{
    "error from broker server", ConsumerErrorType::kBrokerServerError
};

auto const kBrokerServersNotFound = ConsumerErrorTemplate{
    "cannot find brokers", ConsumerErrorType::kBrokerServersNotFound
};



}
}

#endif //ASAPO_CONSUMER_ERROR_H

