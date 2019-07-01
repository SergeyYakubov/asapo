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
    EventHeader(uint64_t file_id_i, uint64_t file_size_i, std::string file_name_i, uint64_t expected_subset_id_i = 0,
                uint64_t expected_subset_size_i = 0 ):
        file_id{file_id_i}, file_size{file_size_i}, file_name{std::move(file_name_i)},
        expected_subset_id{expected_subset_id_i},
        expected_subset_size{expected_subset_size_i} {};
    uint64_t file_id;
    uint64_t file_size;
    std::string file_name;
    uint64_t expected_subset_id;
    uint64_t expected_subset_size;
};

}

#endif //ASAPO_PRODUCER_COMMON_H
