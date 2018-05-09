#ifndef ASAPO_REQUEST_H
#define ASAPO_REQUEST_H

#include "io/io.h"
#include "common/error.h"
#include "receivers_status.h"
#include "common/networking.h"

#include "producer/producer.h"

namespace asapo {

class Request {
  public:
    explicit Request(const asapo::IO* io, const GenericNetworkRequestHeader& header, const void* data,
                     RequestCallback callback);
    Error Send(SocketDescriptor* sd, const ReceiversList& receivers_list);
    const asapo::IO* io__;
  private:
    Error ConnectToReceiver(SocketDescriptor* sd, const std::string& receiver_address);
    Error SendHeaderAndData(SocketDescriptor sd);
    Error ReceiveResponce(SocketDescriptor sd);
    GenericNetworkRequestHeader header_;
    const void* data_;
    RequestCallback callback_;
};
}

#endif //ASAPO_REQUEST_H
