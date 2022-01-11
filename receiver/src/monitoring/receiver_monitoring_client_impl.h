#ifndef ASAPO_RECEIVER_MONITORING_CLIENT_IMPL_H
#define ASAPO_RECEIVER_MONITORING_CLIENT_IMPL_H

#include "receiver_monitoring_client.h"
#include <AsapoMonitoringIngestService.pb.h>
#include "asapo/logger/logger.h"
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <AsapoMonitoringIngestService.grpc.pb.h>
#include <asapo/http_client/http_client.h>
#include <asapo/io/io.h>

namespace asapo {

class ReceiverMonitoringClientImpl : public ReceiverMonitoringClient {
public:
    // Defined below
    class ToBeSendData;
 private:
    std::unique_ptr<std::thread> sendingThread_;

    void SendingThreadFunction();
    Error SendData(ReceiverDataPointContainer* container);

    std::string receiverName_;

    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<AsapoMonitoringIngestService::Stub> client_;

    std::mutex toBeSendData_mutex_;

    SharedCache cache_;
public:
    IO* io__;
    AbstractLogger* log__;
    std::unique_ptr<HttpClient> http_client__;

    std::unique_ptr<ToBeSendData> toBeSendData__;
    mutable bool sendingThreadRunning__ = false;

    explicit ReceiverMonitoringClientImpl(SharedCache cache);
    ReceiverMonitoringClientImpl(const ReceiverMonitoringClientImpl&) = delete;
    ReceiverMonitoringClientImpl& operator=(const ReceiverMonitoringClientImpl&) = delete;

    void StartSendingThread();
    void StopSendingThread();

    void SendProducerToReceiverTransferDataPoint(const std::string& pipelineStepId,
                                                         const std::string& producerInstanceId,
                                                         const std::string& beamtime,
                                                         const std::string& source,
                                                         const std::string& stream,
                                                         uint64_t fileSize,
                                                         uint64_t transferTimeInMicroseconds,
                                                         uint64_t writeIoTimeInMicroseconds,
                                                         uint64_t dbTimeInMicroseconds) override;

    void SendRdsRequestWasMissDataPoint(const std::string& pipelineStepId,
                                                const std::string& consumerInstanceId,
                                                const std::string& beamtime,
                                                const std::string& source,
                                                const std::string& stream) override;

    void SendReceiverRequestDataPoint(const std::string& pipelineStepId,
                                              const std::string& consumerInstanceId,
                                              const std::string& beamtime,
                                              const std::string& source, const std::string& stream,
                                              uint64_t fileSize,
                                              uint64_t transferTimeInMicroseconds) override;

    void FillMemoryStats() override;

private:
    Error ReinitializeClient();
    Error GetMonitoringServerUrl(std::string* url) const;

    static uint64_t WaitTimeMsUntilNextInterval(std::chrono::high_resolution_clock::time_point startTime);

public:
    // Internal struct
    class ToBeSendData {
    public:
        //std::mutex mutex;
        ReceiverDataPointContainer container;
        ASAPO_VIRTUAL ~ToBeSendData()=default;
        ASAPO_VIRTUAL ProducerToReceiverTransferDataPoint* GetProducerToReceiverTransfer(
                const std::string& pipelineStepId,
                const std::string& producerInstanceId,
                const std::string& beamtime,
                const std::string& source,
                const std::string& stream);

        ASAPO_VIRTUAL RdsToConsumerDataPoint* GetReceiverDataServerToConsumer(
                const std::string& pipelineStepId,
                const std::string& consumerInstanceId,
                const std::string& beamtime,
                const std::string& source,
                const std::string& stream);

        ASAPO_VIRTUAL RdsMemoryDataPoint* GetMemoryDataPoint(
                const std::string& beamtime,
                const std::string& source,
                const std::string& stream);
    };
};

}

#endif //ASAPO_RECEIVER_MONITORING_CLIENT_IMPL_H
