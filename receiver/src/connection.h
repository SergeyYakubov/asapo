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

#include "asapo/common/networking.h"
#include "asapo/io/io.h"
#include "request.h"
#include "statistics/receiver_statistics.h"
#include "asapo/logger/logger.h"
#include "request_handler/requests_dispatcher.h"
#include "data_cache.h"

namespace asapo {

class Connection {
  public:
  private:
    std::string address_;
    int socket_fd_;
  public:

    Connection(SocketDescriptor socket_fd, const std::string& address, SharedCache cache, KafkaClient* kafkaClient, std::string receiver_tag);
    ~Connection() = default;

    void Listen() const noexcept;

    std::unique_ptr<IO> io__;
    mutable std::unique_ptr<ReceiverStatistics> statistics__;
    const AbstractLogger* log__;
    std::unique_ptr<RequestsDispatcher> requests_dispatcher__;
  private:
    void ProcessStatisticsAfterRequest(const std::unique_ptr<Request>& request) const noexcept;
};

}


#endif //ASAPO_NetworkProducerPeerImpl_H
