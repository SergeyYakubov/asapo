#ifndef ASAPO_RECEIVER_DATA_SERVER_H
#define ASAPO_RECEIVER_DATA_SERVER_H

#include <memory>

#include "net_server/rds_net_server.h"
#include "request/request_pool.h"
#include "logger/logger.h"
#include "../data_cache.h"
#include "../statistics/statistics.h"

#include "receiver_datacenter_config.h"

namespace asapo {

class ReceiverDataServer {
  private:
    // important to create it before request_pool__
    std::unique_ptr<RequestHandlerFactory> request_handler_factory_;
  public:
    explicit ReceiverDataServer(std::unique_ptr<RdsNetServer> net_server, LogLevel log_level, SharedCache data_cache,
                                const ReceiverDataServerConfig& config);
    std::unique_ptr<RequestPool> request_pool__;
    std::unique_ptr<RdsNetServer> net__;
    const AbstractLogger* log__;
    void Run();
  private:
    SharedCache data_cache_;
    const ReceiverDataServerConfig& config_;
  public:
    std::unique_ptr<Statistics>statistics__;

};

}

#endif //ASAPO_RECEIVER_DATA_SERVER_H
