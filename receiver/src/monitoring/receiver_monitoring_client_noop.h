#ifndef ASAPO_RECEIVER_MONITORING_CLIENT_NOOP_H
#define ASAPO_RECEIVER_MONITORING_CLIENT_NOOP_H

#include "receiver_monitoring_client.h"

namespace asapo {
    class ReceiverMonitoringClientNoop : public ReceiverMonitoringClient {
    public:
        void StartMonitoring() override;
        void SendProducerToReceiverTransferDataPoint(const std::string& pipelineStepId,
                                                     const std::string& producerInstanceId, const std::string& beamtime,
                                                     const std::string& source, const std::string& stream,
                                                     uint64_t fileSize, uint64_t transferTimeInMicroseconds,
                                                     uint64_t writeIoTimeInMicroseconds,
                                                     uint64_t dbTimeInMicroseconds) override;

        void SendRdsRequestWasMissDataPoint(const std::string& pipelineStepId, const std::string& consumerInstanceId,
                                            const std::string& beamtime, const std::string& source,
                                            const std::string& stream) override;

        void SendReceiverRequestDataPoint(const std::string& pipelineStepId, const std::string& consumerInstanceId,
                                          const std::string& beamtime, const std::string& source,
                                          const std::string& stream, uint64_t fileSize,
                                          uint64_t transferTimeInMicroseconds) override;

        void FillMemoryStats() override;
    };
}

#endif //ASAPO_RECEIVER_MONITORING_CLIENT_NOOP_H
