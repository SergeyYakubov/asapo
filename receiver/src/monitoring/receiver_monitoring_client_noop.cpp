
#include <cstdint>
#include "receiver_monitoring_client_noop.h"

void asapo::ReceiverMonitoringClientNoop::SendProducerToReceiverTransferDataPoint(const std::string& pipelineStepId,
                                                                                  const std::string& producerInstanceId,
                                                                                  const std::string& beamtime,
                                                                                  const std::string& source,
                                                                                  const std::string& stream,
                                                                                  uint64_t fileSize,
                                                                                  uint64_t transferTimeInMicroseconds,
                                                                                  uint64_t writeIoTimeInMicroseconds,
                                                                                  uint64_t dbTimeInMicroseconds) {
    (void)(pipelineStepId);
    (void)(producerInstanceId);
    (void)(beamtime);
    (void)(source);
    (void)(stream);
    (void)(fileSize);
    (void)(transferTimeInMicroseconds);
    (void)(writeIoTimeInMicroseconds);
    (void)(dbTimeInMicroseconds);
}

void asapo::ReceiverMonitoringClientNoop::SendRdsRequestWasMissDataPoint(const std::string& pipelineStepId,
                                                                         const std::string& consumerInstanceId,
                                                                         const std::string& beamtime,
                                                                         const std::string& source,
                                                                         const std::string& stream) {
    (void)(pipelineStepId);
    (void)(consumerInstanceId);
    (void)(beamtime);
    (void)(source);
    (void)(stream);
}

void asapo::ReceiverMonitoringClientNoop::SendReceiverRequestDataPoint(const std::string& pipelineStepId,
                                                                       const std::string& consumerInstanceId,
                                                                       const std::string& beamtime,
                                                                       const std::string& source,
                                                                       const std::string& stream, uint64_t fileSize,
                                                                       uint64_t transferTimeInMicroseconds) {
    (void)(pipelineStepId);
    (void)(consumerInstanceId);
    (void)(beamtime);
    (void)(source);
    (void)(stream);
    (void)(fileSize);
    (void)(transferTimeInMicroseconds);
}

void asapo::ReceiverMonitoringClientNoop::FillMemoryStats() {

}

void asapo::ReceiverMonitoringClientNoop::StartMonitoring() {

}
