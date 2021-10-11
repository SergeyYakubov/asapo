#include "receiver_prometheus_metrics.h"

namespace  asapo {

std::string ReceiverPrometheusMetrics::Metrics() const {
    return "alive 1\n";
}

}


