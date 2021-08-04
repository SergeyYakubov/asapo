#ifndef ASAPO_RECEIVER_MONITORING_CLIENT_H
#define ASAPO_RECEIVER_MONITORING_CLIENT_H

#include <AsapoMonitoringIngestService.pb.h>
#include "asapo/logger/logger.h"
#include "../data_cache.h"
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <AsapoMonitoringIngestService.grpc.pb.h>
#include <asapo/http_client/http_client.h>

namespace asapo {

class ReceiverMonitoringClient {
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

    static std::chrono::high_resolution_clock::time_point HelperTimeNow();
    static uint64_t HelperTimeDiffInMicroseconds(std::chrono::high_resolution_clock::time_point startTime);

    //ReceiverMonitoringClient() = ONLY_IN_TESTS_CTOR;
    ReceiverMonitoringClient(SharedCache cache);
    ReceiverMonitoringClient(const ReceiverMonitoringClient&) = delete;
    ReceiverMonitoringClient& operator=(const ReceiverMonitoringClient&) = delete;

    VIRTUAL void StartSendingThread();
    VIRTUAL void StopSendingThread();

    VIRTUAL void SendProducerToReceiverTransferDataPoint(
            const std::string& pipelineStepId,
            const std::string& producerInstanceId,

            const std::string& beamtime,
            const std::string& source,
            const std::string& stream,
            const std::string& fileName,

            uint64_t fileSize,
            uint64_t transferTimeInMicroseconds,
            uint64_t writeIoTimeInMicroseconds,
            uint64_t dbTimeInMicroseconds
            );

    VIRTUAL void SendRdsRequestWasMissDataPoint(
            const std::string& pipelineStepId,
            const std::string& consumerInstanceId,

            const std::string& beamtime,
            const std::string& source,
            const std::string& stream,
            const std::string& fileName
            );

    VIRTUAL void SendReceiverRequestDataPoint(
            const std::string& pipelineStepId,
            const std::string& consumerInstanceId,

            const std::string& beamtime,
            const std::string& source,
            const std::string& stream,
            const std::string& fileName,

            uint64_t fileSize,
            uint64_t transferTimeInMicroseconds
            );

    VIRTUAL void FillMemoryStats();

private:
    Error GetMonitoringServerUrl(std::string* url) const;

public:
    // Internal struct
    class ToBeSendData {
    public:
        //std::mutex mutex;
        ReceiverDataPointContainer container;

        VIRTUAL ProducerToReceiverTransferDataPoint* GetProducerToReceiverTransfer(
                const std::string& pipelineStepId,
                const std::string& producerInstanceId,
                const std::string& beamtime,
                const std::string& source,
                const std::string& stream);

        VIRTUAL RdsToConsumerDataPoint* GetReceiverDataServerToConsumer(
                const std::string& pipelineStepId,
                const std::string& consumerInstanceId,
                const std::string& beamtime,
                const std::string& source,
                const std::string& stream);

        VIRTUAL RdsMemoryDataPoint* GetMemoryDataPoint(
                const std::string& beamtime,
                const std::string& source,
                const std::string& stream);
    };

    Error ReinitializeClient();
};

using SharedReceiverMonitoringClient = std::shared_ptr<asapo::ReceiverMonitoringClient>;
}

#endif //ASAPO_RECEIVER_MONITORING_CLIENT_H
