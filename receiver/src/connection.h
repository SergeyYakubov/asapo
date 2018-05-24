#ifndef ASAPO_NetworkProducerPeerImpl_H
#define ASAPO_NetworkProducerPeerImpl_H

#include "connection.h"

#include <string>
#include <map>
#include <utility>
#include <thread>
#include <iostream>
#include <atomic>
#include <vector>

#include "common/networking.h"
#include "io/io.h"
#include "request.h"
#include "statistics.h"
#include "logger/logger.h"

namespace asapo {

class Connection {
  public:
  private:
    uint32_t connection_id_;
    std::string address_;
    int socket_fd_;
  public:
    static size_t kRequestHandlerMaxBufferSize;
    static std::atomic<uint32_t> kNetworkProducerPeerImplGlobalCounter;

    Connection(SocketDescriptor socket_fd, const std::string& address,std::string receiver_tag);
    ~Connection() = default;

    void Listen() const noexcept;
    uint64_t GetId() const noexcept;

    std::unique_ptr<RequestFactory> request_factory__;
    std::unique_ptr<IO> io__;
    mutable std::unique_ptr<Statistics> statistics__;
    const AbstractLogger* log__;
  private:
    std::unique_ptr<Request> WaitForNewRequest(Error* err) const noexcept;
    Error ProcessRequest(const std::unique_ptr<Request>& request) const noexcept;
    void ProcessStatisticsAfterRequest(const std::unique_ptr<Request>& request) const noexcept;
};

}


#endif //ASAPO_NetworkProducerPeerImpl_H
