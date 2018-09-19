#ifndef ASAPO_PRODUCER_REQUEST_H
#define ASAPO_PRODUCER_REQUEST_H

#include "common/networking.h"
#include "producer/common.h"
#include "common/data_structs.h"
#include "io/io.h"

namespace asapo {

struct Request {
    std::string beamtime_id;
    GenericRequestHeader header;
    FileData data;
    std::string original_filepath;
    RequestCallback callback;
    Error ReadDataFromFileIfNeeded(const IO* io);
};

}

#endif //ASAPO_PRODUCER_REQUEST_H
