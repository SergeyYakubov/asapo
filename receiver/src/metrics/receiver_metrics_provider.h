#ifndef ASAPO_RECEIVER_SRC_METRICS_RECEIVER_METRICS_PROVIDER_H_
#define ASAPO_RECEIVER_SRC_METRICS_RECEIVER_METRICS_PROVIDER_H_

#include <string>

namespace asapo {

class ReceiverMetricsProvider {
  public:
    virtual std::string Metrics() const = 0;
    virtual ~ReceiverMetricsProvider() = default;
};


}



#endif //ASAPO_RECEIVER_SRC_METRICS_RECEIVER_METRICS_PROVIDER_H_
