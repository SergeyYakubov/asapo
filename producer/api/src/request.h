#ifndef ASAPO_PRODUCER_REQUEST_H
#define ASAPO_PRODUCER_REQUEST_H

#include "common/networking.h"
#include "producer/common.h"
#include "common/data_structs.h"

namespace asapo {

struct Request {
    std::string beamtime_id;
    GenericRequestHeader header;
    FileData data;
    RequestCallback callback;
};

}

#endif //ASAPO_PRODUCER_REQUEST_H
