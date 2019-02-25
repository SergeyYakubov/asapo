#ifndef ASAPO_RECEIVER_DATA_SERVER_H
#define ASAPO_RECEIVER_DATA_SERVER_H

#include <memory>

#include "net_server.h"
#include "request/request_pool.h"
#include "logger/logger.h"
#include "../data_cache.h"

namespace asapo {

class ReceiverDataServer {
  private:
    // important to create it before request_pool__
    std::unique_ptr<RequestHandlerFactory> request_handler_factory_;
  public:
    explicit ReceiverDataServer(std::string address, LogLevel log_level, uint8_t n_threads, SharedCache data_cache);
    std::unique_ptr<RequestPool> request_pool__;
    std::unique_ptr<NetServer> net__;
    const AbstractLogger* log__;
    void Run();
  private:
    SharedCache data_cache_;

};

}

#endif //ASAPO_RECEIVER_DATA_SERVER_H
