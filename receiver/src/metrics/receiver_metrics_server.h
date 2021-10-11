#ifndef ASAPO_RECEIVER_SRC_METRICS_RECEIVER_METRICS_SERVER_H_
#define ASAPO_RECEIVER_SRC_METRICS_RECEIVER_METRICS_SERVER_H_

#include "asapo/common/error.h"

#include <thread>
#include <memory>

#include "receiver_metrics_provider.h"

namespace asapo {

class ReceiverMetricsServer {
  public:
    virtual void ListenAndServe(std::string port, std::unique_ptr<ReceiverMetricsProvider> provider)  = 0;
    virtual ~ReceiverMetricsServer() = default;
};

}

#endif //ASAPO_RECEIVER_SRC_METRICS_RECEIVER_METRICS_SERVER_H_
