#include "receiver_prometheus_metrics.h"

namespace  asapo {

std::string ReceiverPrometheusMetrics::Metrics() const {
    return R"(
# HELP alive whether receiver is up
# TYPE alive gauge
alive 1
)";
}

}


