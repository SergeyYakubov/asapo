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


struct EventHeader {
    uint64_t file_id;
    uint64_t file_size;
    std::string file_name;
};

}

#endif //ASAPO_PRODUCER_COMMON_H
