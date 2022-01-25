#ifndef ASAPO_REQUESTS_DISPATCHER_H
#define ASAPO_REQUESTS_DISPATCHER_H

#include "asapo/preprocessor/definitions.h"
#include "asapo/common/error.h"
#include "../request.h"
#include "request_factory.h"
#include "asapo/io/io.h"
#include "../statistics/receiver_statistics.h"
#include "asapo/logger/logger.h"
#include "../data_cache.h"

namespace asapo {

class RequestsDispatcher {
  public:
    RequestsDispatcher(SocketDescriptor socket_fd, std::string address, ReceiverStatistics* statistics,SharedReceiverMonitoringClient monitoring, SharedCache cache, KafkaClient* kafka_client);
    ASAPO_VIRTUAL Error ProcessRequest(const std::unique_ptr<Request>& request) const noexcept;
    ASAPO_VIRTUAL std::unique_ptr<Request> GetNextRequest(Error* err) const noexcept;
    ASAPO_VIRTUAL ~RequestsDispatcher() = default;
    ReceiverStatistics* statistics__;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
    std::unique_ptr<RequestFactory> request_factory__;
  private:
    SocketDescriptor socket_fd_;
    std::string producer_uri_;
    GenericNetworkResponse CreateResponseToRequest(const std::unique_ptr<Request>& request,
                                                   const Error& handle_error) const;
    Error HandleRequest(const std::unique_ptr<Request>& request) const;
    Error SendResponse(const std::unique_ptr<Request>& request, const Error& handle_error) const;
};

}

#endif //ASAPO_REQUESTS_DISPATCHER_H
