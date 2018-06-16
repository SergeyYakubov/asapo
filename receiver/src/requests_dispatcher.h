#ifndef ASAPO_REQUESTS_DISPATCHER_H
#define ASAPO_REQUESTS_DISPATCHER_H

#include "preprocessor/definitions.h"
#include "common/error.h"
#include "request.h"
#include "io/io.h"
#include "statistics.h"
#include "logger/logger.h"
#include "connection_authorizer.h"

namespace asapo {

class RequestsDispatcher {
 public:
  RequestsDispatcher(SocketDescriptor socket_fd, std::string address, Statistics* statistics);
  VIRTUAL Error ProcessRequest(const std::unique_ptr<Request>& request) const noexcept;
  VIRTUAL std::unique_ptr<Request> GetNextRequest(Error* err) const noexcept;
  Statistics* statistics__;
  std::unique_ptr<IO> io__;
  const AbstractLogger* log__;
  std::unique_ptr<RequestFactory> request_factory__;
  std::unique_ptr<ConnectionAuthorizer>authorizer__;
 private:
  SocketDescriptor socket_fd_;
  std::string producer_uri_;
};

}

#endif //ASAPO_REQUESTS_DISPATCHER_H


/*
    mutable bool auth_header_was_read_ = false;
    Error ReadAuthorizationHeaderIfNeeded() const;
    Error SendAuthorizationResponseIfNeeded(const Error& auth_err) const;
    Error AuthorizeIfNeeded() const;
    std::unique_ptr<Request> WaitForNewRequest(Error* err) const noexcept;
    Error ProcessRequest(const std::unique_ptr<Request>& request) const noexcept;
    void ProcessStatisticsAfterRequest(const std::unique_ptr<Request>& request) const noexcept;
    mutable std::string beamtime_id_;

 */