#ifndef ASAPO_PRODUCER_COMMON_H
#define ASAPO_PRODUCER_COMMON_H

#include <cstdint>
#include <functional>

#include "asapo/common/networking.h"
#include "asapo/common/error.h"

namespace asapo {

const uint8_t kMaxProcessingThreads = 32;


struct RequestCallbackPayload {
    GenericRequestHeader original_header;
    MessageData data;
    std::string response;
};

using RequestCallback =  std::function<void(RequestCallbackPayload, Error)>;

enum class RequestHandlerType {
    kTcp,
    kFilesystem
};


struct MessageHeader {
    MessageHeader() {};
    MessageHeader(uint64_t message_id_i, uint64_t data_size_i, std::string file_name_i,
                  std::string user_metadata_i = "",
                  uint64_t dataset_substream_i = 0,
                  uint64_t dataset_size_i = 0 ):
        message_id{message_id_i}, data_size{data_size_i},
        file_name{std::move(file_name_i)},
        user_metadata{std::move(user_metadata_i)},
        dataset_substream{dataset_substream_i},
        dataset_size{dataset_size_i} {};
    uint64_t message_id = 0;
    uint64_t data_size = 0;
    std::string file_name;
    std::string user_metadata;
    uint64_t dataset_substream = 0;
    uint64_t dataset_size = 0;
};

}

#endif //ASAPO_PRODUCER_COMMON_H
