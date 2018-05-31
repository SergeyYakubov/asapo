#ifndef ASAPO_PRODUCER_COMMON_H
#define ASAPO_PRODUCER_COMMON_H

#include <cstdint>
#include <functional>

#include "common/networking.h"
#include "common/error.h"

namespace asapo {

const uint8_t kMaxProcessingThreads = 32;

using RequestCallback =  std::function<void(GenericRequestHeader, Error)>;

enum class RequestHandlerType {
    kTcp,
    kFilesystem
};


}

#endif //ASAPO_PRODUCER_COMMON_H
