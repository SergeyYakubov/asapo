#ifndef ASAPO_PRODUCER_REQUEST_H
#define ASAPO_PRODUCER_REQUEST_H

#include "common/networking.h"
#include "producer/common.h"

namespace asapo {

struct Request {
    GenericRequestHeader header;
    const void* data;
    RequestCallback callback;
};

}

#endif //ASAPO_PRODUCER_REQUEST_H
