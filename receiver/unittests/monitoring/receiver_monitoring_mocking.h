#ifndef ASAPO_RECEIVER_MONITORING_MOCKING_H
#define ASAPO_RECEIVER_MONITORING_MOCKING_H

#include "../../src/monitoring/receiver_monitoring_client.h"
#ifdef NEW_RECEIVER_MONITORING_ENABLED
#include "../../src/monitoring/receiver_monitoring_client_impl.h"
#endif

namespace asapo {

class MockReceiverMonitoringClient : public asapo::ReceiverMonitoringClient {
public:
    MOCK_METHOD9(SendProducerToReceiverTransferDataPoint, void(const std::string& pipelineStepId, const std::string& producerInstanceId,
            const std::string& beamtime, const std::string& source,
            const std::string& stream, uint64_t fileSize,
            uint64_t transferTimeInMicroseconds, uint64_t writeIoTimeInMicroseconds,
            uint64_t dbTimeInMicroseconds));

    MOCK_METHOD5(SendRdsRequestWasMissDataPoint, void(const std::string& pipelineStepId, const std::string& consumerInstanceId,
            const std::string& beamtime, const std::string& source,
            const std::string& stream));

    MOCK_METHOD7(SendReceiverRequestDataPoint, void(const std::string& pipelineStepId, const std::string& consumerInstanceId,
            const std::string& beamtime, const std::string& source, const std::string& stream,
            uint64_t fileSize, uint64_t transferTimeInMicroseconds));

    MOCK_METHOD0(FillMemoryStats, void());
};

#ifdef NEW_RECEIVER_MONITORING_ENABLED
class MockReceiverMonitoringClientImpl_ToBeSendData : public asapo::ReceiverMonitoringClientImpl::ToBeSendData {
public:
    MOCK_METHOD5(GetProducerToReceiverTransfer, ProducerToReceiverTransferDataPoint* (
            const std::string& pipelineStepId,
            const std::string& producerInstanceId,
            const std::string& beamtime,
            const std::string& source,
            const std::string& stream));

    MOCK_METHOD5(GetReceiverDataServerToConsumer, RdsToConsumerDataPoint* (
            const std::string& pipelineStepId,
            const std::string& producerInstanceId,
            const std::string& beamtime,
            const std::string& source,
            const std::string& stream
            ));

    MOCK_METHOD3(GetMemoryDataPoint, RdsMemoryDataPoint* (
            const std::string& beamtime,
            const std::string& source,
            const std::string& stream
            ));

};
#endif

}

#endif //ASAPO_RECEIVER_MONITORING_MOCKING_H
