#ifndef ASAPO_RECEIVER_DATA_SERVER_H
#define ASAPO_RECEIVER_DATA_SERVER_H

#include <memory>

#include "net_server.h"
#include "request_pool.h"
#include "logger/logger.h"

namespace asapo {

class ReceiverDataServer {
  public:
    explicit ReceiverDataServer(std::string address);
    std::unique_ptr<RequestPool> request_pool__;
    std::unique_ptr<NetServer> net__;
    const AbstractLogger* log__;
    void Run();
  private:
};

}

#endif //ASAPO_RECEIVER_DATA_SERVER_H
