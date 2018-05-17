#ifndef ASAPO_PRODUCER_REQUEST_H
#define ASAPO_PRODUCER_REQUEST_H

#include "common/networking.h"
#include "producer/producer.h"

namespace asapo {

struct Request {
  GenericNetworkRequestHeader header;
  const void* data;
  RequestCallback callback;
  uint64_t GetMemoryRequitements() {
      return header.data_size + sizeof(Request);
  }

};

}

#endif //ASAPO_PRODUCER_REQUEST_H
