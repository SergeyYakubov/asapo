#ifndef ASAPO_RECEIVER_SRC_METRICS_RECEIVER_PROMETHEUS_METRICS_H_
#define ASAPO_RECEIVER_SRC_METRICS_RECEIVER_PROMETHEUS_METRICS_H_

#include "receiver_metrics_provider.h"

namespace asapo {

class ReceiverPrometheusMetrics final : public ReceiverMetricsProvider {
  public:
    std::string Metrics() const override;

};

}

#endif //ASAPO_RECEIVER_SRC_METRICS_RECEIVER_PROMETHEUS_METRICS_H_
