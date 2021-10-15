#ifndef ASAPO_RECEIVER_MONITORING_CLIENT_H
#define ASAPO_RECEIVER_MONITORING_CLIENT_H

#include <chrono>
#include <string>
#include <memory>
#include "../data_cache.h"

namespace asapo {

class ReceiverMonitoringClient {
protected:
    ReceiverMonitoringClient() = default;
public:
    static std::chrono::high_resolution_clock::time_point HelperTimeNow();
    static uint64_t HelperTimeDiffInMicroseconds(std::chrono::high_resolution_clock::time_point startTime);

    ReceiverMonitoringClient(const ReceiverMonitoringClient&) = delete;
    ReceiverMonitoringClient& operator=(const ReceiverMonitoringClient&) = delete;

    virtual void SendProducerToReceiverTransferDataPoint(const std::string& pipelineStepId,
                                                         const std::string& producerInstanceId,
                                                         const std::string& beamtime,
                                                         const std::string& source,
                                                         const std::string& stream,
                                                         uint64_t fileSize,
                                                         uint64_t transferTimeInMicroseconds,
                                                         uint64_t writeIoTimeInMicroseconds,
                                                         uint64_t dbTimeInMicroseconds) = 0;

    virtual void SendRdsRequestWasMissDataPoint(const std::string& pipelineStepId,
                                                const std::string& consumerInstanceId,
                                                const std::string& beamtime,
                                                const std::string& source,
                                                const std::string& stream) = 0;

    virtual void SendReceiverRequestDataPoint(const std::string& pipelineStepId,
                                              const std::string& consumerInstanceId,
                                              const std::string& beamtime,
                                              const std::string& source, const std::string& stream,
                                              uint64_t fileSize,
                                              uint64_t transferTimeInMicroseconds) = 0;

    virtual void FillMemoryStats() = 0;
};

using SharedReceiverMonitoringClient = std::shared_ptr<asapo::ReceiverMonitoringClient>;

SharedReceiverMonitoringClient GenerateDefaultReceiverMonitoringClient(const SharedCache& cache, bool forceNoopImplementation);

}
#endif //ASAPO_RECEIVER_MONITORING_CLIENT_H
