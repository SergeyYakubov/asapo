#include "receiver_monitoring_client.h"
#include "receiver_monitoring_client_noop.h"

#ifdef NEW_RECEIVER_MONITORING_ENABLED
#include "receiver_monitoring_client_impl.h"
#endif

using namespace asapo;

SharedReceiverMonitoringClient asapo::GenerateDefaultReceiverMonitoringClient(const SharedCache& cache, bool forceNoopImplementation) {
#ifdef NEW_RECEIVER_MONITORING_ENABLED
    if (!forceNoopImplementation) {
        return SharedReceiverMonitoringClient{new ReceiverMonitoringClientImpl{cache}};
    }
#else
    (void)(cache);
    (void)(forceNoopImplementation);
#endif
    return SharedReceiverMonitoringClient{new ReceiverMonitoringClientNoop};
}



std::chrono::high_resolution_clock::time_point asapo::ReceiverMonitoringClient::HelperTimeNow() {
    return std::chrono::high_resolution_clock::now();
}

uint64_t asapo::ReceiverMonitoringClient::HelperTimeDiffInMicroseconds(std::chrono::high_resolution_clock::time_point startTime) {
    auto now = HelperTimeNow();
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count());
}
