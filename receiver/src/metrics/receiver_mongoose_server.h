#ifndef ASAPO_RECEIVER_SRC_METRICS_RECEIVER_MONGOOSE_SERVER_H_
#define ASAPO_RECEIVER_SRC_METRICS_RECEIVER_MONGOOSE_SERVER_H_

#include "receiver_metrics_server.h"

namespace asapo {
class AbstractLogger;

class ReceiverMongooseServer final :  public ReceiverMetricsServer {
  public:
    ReceiverMongooseServer();
    void ListenAndServe(std::string port, std::unique_ptr<ReceiverMetricsProvider> provider) override;
  private:
    const AbstractLogger* log__;
};

}



#endif //ASAPO_RECEIVER_SRC_METRICS_RECEIVER_MONGOOSE_SERVER_H_
