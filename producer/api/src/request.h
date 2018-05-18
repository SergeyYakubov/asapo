#ifndef ASAPO_PRODUCER_REQUEST_H
#define ASAPO_PRODUCER_REQUEST_H

#include "common/networking.h"
#include "producer/producer.h"

namespace asapo {

struct Request {
  GenericNetworkRequestHeader header;
  const void* data;
  RequestCallback callback;
};

}

#endif //ASAPO_PRODUCER_REQUEST_H
