#ifndef ASAPO_REQUEST_H
#define ASAPO_REQUEST_H

#include "io/io.h"
#include "common/error.h"
#include "receiver_discovery_service.h"
#include "common/networking.h"

#include "producer/producer.h"

#ifdef UNIT_TESTS
#define VIRTUAL virtual
#endif


namespace asapo {

class Request {
  public:
    explicit Request(const GenericNetworkRequestHeader& header, const void* data,
                     RequestCallback callback);
    VIRTUAL Error Send(SocketDescriptor* sd, const ReceiversList& receivers_list, bool rebalance);
    VIRTUAL ~Request() = default;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
    uint64_t GetMemoryRequitements();
  private:
    Error ConnectToReceiver(SocketDescriptor* sd, const std::string& receiver_address);
    Error SendHeaderAndData(SocketDescriptor sd, const std::string& receiver_address);
    Error ReceiveResponse(SocketDescriptor sd, const std::string& receiver_address);
    Error TrySendToReceiver(SocketDescriptor sd, const std::string& receiver_address);
    GenericNetworkRequestHeader header_;
    const void* data_;
    RequestCallback callback_;
};
}

#endif //ASAPO_REQUEST_H
