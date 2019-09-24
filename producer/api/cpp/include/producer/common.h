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
    EventHeader() {};
    EventHeader(uint64_t file_id_i, uint64_t file_size_i, std::string file_name_i,
                std::string user_metadata_i = "",
                uint64_t subset_id_i = 0,
                uint64_t subset_size_i = 0 ):
        file_id{file_id_i}, file_size{file_size_i},
        file_name{std::move(file_name_i)},
        user_metadata{std::move(user_metadata_i)},
        subset_id{subset_id_i},
        subset_size{subset_size_i} {};
    uint64_t file_id = 0;
    uint64_t file_size = 0;
    std::string file_name;
    std::string user_metadata;
    uint64_t subset_id = 0;
    uint64_t subset_size = 0;
};

}

#endif //ASAPO_PRODUCER_COMMON_H