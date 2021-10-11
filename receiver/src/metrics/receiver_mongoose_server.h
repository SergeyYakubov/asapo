#ifndef ASAPO_RECEIVER_SRC_METRICS_RECEIVER_MONGOOSE_SERVER_H_
#define ASAPO_RECEIVER_SRC_METRICS_RECEIVER_MONGOOSE_SERVER_H_

#include "receiver_metrics_server.h"

namespace asapo {
class ReceiverMongooseServer final :  public ReceiverMetricsServer {
  public:
    Error ListenAndServe(std::string port, std::unique_ptr<ReceiverMetricsProvider> provider) override;

};

}



#endif //ASAPO_RECEIVER_SRC_METRICS_RECEIVER_MONGOOSE_SERVER_H_
