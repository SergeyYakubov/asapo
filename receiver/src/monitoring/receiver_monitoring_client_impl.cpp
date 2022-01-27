#include <chrono>
#include <utility>
#include <asapo/io/io_factory.h>
#include "receiver_monitoring_client_impl.h"
#include "../receiver_logger.h"
#include "./AsapoMonitoringIngestService.pb.h"
#include "../receiver_config.h"
#include "../receiver_error.h"

using namespace asapo;

static const int kUniversalSendingIntervalMs = 5000;

uint64_t NowUnixTimestampMs() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}

asapo::ReceiverMonitoringClientImpl::ReceiverMonitoringClientImpl(asapo::SharedCache cache) : cache_(cache), io__{GenerateDefaultIO()}, log__{GetDefaultReceiverMonitoringLogger()}, http_client__{DefaultHttpClient()}, toBeSendData__{
    new ToBeSendData
}
{
    Error err;
    std::string hostname = io__->GetHostName(&err);
    if (err) {
        hostname = "hostnameerror";
    }
    receiverName_ = "receiver_" +  hostname + "_" + std::to_string(io__->GetCurrentPid());


}

void ReceiverMonitoringClientImpl::StartMonitoring() {
    ReinitializeClient();
    StartSendingThread();
}


void asapo::ReceiverMonitoringClientImpl::StartSendingThread() {
    if (sendingThreadRunning__) {
        return;
    }

    if (!client_) {
        log__->Warning("Tried to start monitoring sending thread, but client is not successfully initialized");
        return;
    }

    log__->Info("Starting sending thread");
    sendingThreadRunning__ = true;
    sendingThread_ = std::unique_ptr<std::thread>(io__->NewThread("MonitoringSender", [this]() {
        SendingThreadFunction();
    }));
}

void asapo::ReceiverMonitoringClientImpl::StopSendingThread() {
    if (!sendingThreadRunning__) {
        log__->Info("sending thread already stopped");
        return;
    }

    log__->Info("Stopping sending thread");
    sendingThreadRunning__ = false;
    if (sendingThread_.get() && sendingThread_->joinable()) {
        sendingThread_->join();
        sendingThread_=nullptr;
    }
    log__->Info("Stopped sending thread");
}

void asapo::ReceiverMonitoringClientImpl::SendingThreadFunction() {
    auto globalStartTime = ReceiverMonitoringClient::HelperTimeNow();

    std::unique_ptr<ToBeSendData> localToBeSend{new ToBeSendData};
    while(sendingThreadRunning__) {
        auto iterationStart = ReceiverMonitoringClient::HelperTimeNow();
        FillMemoryStats();

        // Clear and swap data
        localToBeSend->container.Clear();
        {
            std::lock_guard<std::mutex> lockGuard(toBeSendData_mutex_);
            toBeSendData__.swap(localToBeSend);
        }

        Error err = SendData(&(localToBeSend->container));
        if (err) {
            log__->Info("Sending of all monitoring data failed: " + err->Explain());
            ReinitializeClient();

            err = SendData(&(localToBeSend->container));
            if (err) {
                log__->Info("Sending of all monitoring data failed AGAIN. Discarding data " + err->Explain());
            }
        }

        size_t size = localToBeSend->container.ByteSizeLong();

        uint64_t sleepDurationInMs = WaitTimeMsUntilNextInterval(globalStartTime);
        auto tookTimeMs = ReceiverMonitoringClient::HelperTimeDiffInMicroseconds(iterationStart) / 1000;

        if (tookTimeMs > kUniversalSendingIntervalMs) {
            sleepDurationInMs = 0;
        }

        auto tookInMsStr = std::to_string(tookTimeMs);

        if (!err) {
            log__->Debug("Sending of all monitoring data(" + std::to_string(size) + " byte) took " + tookInMsStr+ "ms (sleeping for " + std::to_string(sleepDurationInMs) + "ms)");
        } else {
            log__->Info("Error took " + tookInMsStr + "ms, will try again in " + std::to_string(sleepDurationInMs) + "ms");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationInMs));
    }
}

void asapo::ReceiverMonitoringClientImpl::SendProducerToReceiverTransferDataPoint(const std::string& pipelineStepId,
                                                                              const std::string& producerInstanceId,
                                                                              const std::string& beamtime,
                                                                              const std::string& source,
                                                                              const std::string& stream,
                                                                              uint64_t fileSize,
                                                                              uint64_t transferTimeInMicroseconds,
                                                                              uint64_t writeIoTimeInMicroseconds,
                                                                              uint64_t dbTimeInMicroseconds) {
    if (!sendingThreadRunning__) {
        return;
    }

    { // LockGuard block
        std::lock_guard<std::mutex> lockGuard(toBeSendData_mutex_);

        auto dataPoint = toBeSendData__->GetProducerToReceiverTransfer(pipelineStepId, producerInstanceId, beamtime, source, stream);

        dataPoint->set_filecount(dataPoint->filecount() + 1);
        dataPoint->set_totalfilesize(dataPoint->totalfilesize() + fileSize);

        dataPoint->set_totaltransferreceivetimeinmicroseconds(dataPoint->totaltransferreceivetimeinmicroseconds() + transferTimeInMicroseconds);
        dataPoint->set_totalwriteiotimeinmicroseconds(dataPoint->totalwriteiotimeinmicroseconds() + writeIoTimeInMicroseconds);
        dataPoint->set_totaldbtimeinmicroseconds(dataPoint->totaldbtimeinmicroseconds() + dbTimeInMicroseconds);
    }
}

void asapo::ReceiverMonitoringClientImpl::SendRdsRequestWasMissDataPoint(const std::string& pipelineStepId,
                                                                     const std::string& consumerInstanceId,
                                                                     const std::string& beamtime,
                                                                     const std::string& source,
                                                                     const std::string& stream) {
    if (!sendingThreadRunning__) {
        return;
    }

    { // LockGuard block
        std::lock_guard<std::mutex> lockGuard(toBeSendData_mutex_);

        auto dataPoint = toBeSendData__->GetReceiverDataServerToConsumer(pipelineStepId, consumerInstanceId, beamtime, source, stream);

        dataPoint->set_misses(dataPoint->misses() + 1);
    }
}

void asapo::ReceiverMonitoringClientImpl::SendReceiverRequestDataPoint(const std::string& pipelineStepId,
                                                                   const std::string& consumerInstanceId,
                                                                   const std::string& beamtime,
                                                                   const std::string& source, const std::string& stream,
                                                                   uint64_t fileSize,
                                                                   uint64_t transferTimeInMicroseconds) {
    if (!sendingThreadRunning__) {
        return;
    }

    { // LockGuard block
        std::lock_guard<std::mutex> lockGuard(toBeSendData_mutex_);

        auto dataPoint = toBeSendData__->GetReceiverDataServerToConsumer(pipelineStepId, consumerInstanceId, beamtime, source, stream);

        dataPoint->set_hits(dataPoint->hits() + 1);
        dataPoint->set_totalfilesize(dataPoint->totalfilesize() + fileSize);
        dataPoint->set_totaltransfersendtimeinmicroseconds(dataPoint->totaltransfersendtimeinmicroseconds() + transferTimeInMicroseconds);
    }
}

void asapo::ReceiverMonitoringClientImpl::FillMemoryStats() {
    if (cache_ == nullptr) {
        return;
    }

    auto metaVector = cache_->AllMetaInfosAsVector();

    {
        std::lock_guard<std::mutex> lockGuard(toBeSendData_mutex_);
        for (const auto& meta : metaVector) {
            auto mDataPoint = toBeSendData__->GetMemoryDataPoint(meta->beamtime, meta->source, meta->stream);
            mDataPoint->set_usedbytes(mDataPoint->usedbytes() + meta->size);
        }

        for (auto& group : *(toBeSendData__->container.mutable_groupedmemorystats())) {
            group.set_totalbytes(cache_->GetCacheSize());
        }
    }
}

asapo::Error asapo::ReceiverMonitoringClientImpl::SendData(ReceiverDataPointContainer* container) {
    container->set_receivername(receiverName_);

    uint64_t baseTimestamp = NowUnixTimestampMs();
    container->set_timestampms(baseTimestamp);

    grpc::ClientContext context;
    Empty response;

    auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
    context.set_deadline(deadline);
    grpc::Status status = client_->InsertReceiverDataPoints(&context, *container, &response);

    if (!status.ok()) {
        return GeneralErrorTemplates::kSimpleError.Generate("Monitoring gRPC Send Error " + status.error_message() + " Details: " + status.error_details());
    }

    return nullptr;
}

asapo::Error asapo::ReceiverMonitoringClientImpl::ReinitializeClient() {
    std::string url;
    Error err = GetMonitoringServerUrl(&url);
    if (err) {
        return err;
    }

    log__->Debug("MonitoringServer address: '" + url + "'");

    auto newChannel = std::shared_ptr<grpc::Channel>(grpc::CreateChannel(url, grpc::InsecureChannelCredentials()));
    auto newClient = std::unique_ptr<AsapoMonitoringIngestService::Stub>(AsapoMonitoringIngestService::NewStub(newChannel));

    // TODO: verify connection
    {
        client_.swap(newClient);
        channel_.swap(newChannel);

        newClient.reset(); // delete client first
        newChannel.reset();
    }
    return nullptr;
}

asapo::Error asapo::ReceiverMonitoringClientImpl::GetMonitoringServerUrl(std::string* url) const {
    if (GetReceiverConfig()->monitoring_server_url != "auto") {
        *url = GetReceiverConfig()->monitoring_server_url;
        return nullptr;
    }

    HttpCode code;
    Error http_err;
    *url = http_client__->Get(GetReceiverConfig()->discovery_server + "/asapo-monitoring", &code, &http_err);
    if (http_err) {
        log__->Error(
                std::string{"http error while trying to discover monitoring server from"} + GetReceiverConfig()->discovery_server
                + " : " + http_err->Explain());
        return ReceiverErrorTemplates::kInternalServerError.Generate("http error while trying to discover monitoring server" +
        http_err->Explain());
    }

    if (code != HttpCode::OK) {
        log__->Error(
                std::string{"http status code error while trying to discover monitoring server from "} + GetReceiverConfig()->discovery_server
                + " : http code" + std::to_string((int) code));
        return ReceiverErrorTemplates::kInternalServerError.Generate("http status code error while trying to discover monitoring server");
    }

    return nullptr;
}

ProducerToReceiverTransferDataPoint*
asapo::ReceiverMonitoringClientImpl::ToBeSendData::GetProducerToReceiverTransfer(const std::string& pipelineStepId,
                                                                             const std::string& producerInstanceId,
                                                                             const std::string& beamtime,
                                                                             const std::string& source,
                                                                             const std::string& stream) {

    for (auto& item : *(container.mutable_groupedp2rtransfers())) {
        if (item.pipelinestepid() == pipelineStepId &&
        item.producerinstanceid() == producerInstanceId &&
        item.beamtime() == beamtime &&
        item.source() == source &&
        item.stream() == stream) {
            return &item;
        }
    }

    auto newItem = container.mutable_groupedp2rtransfers()->Add();

    *(newItem->mutable_pipelinestepid()) = pipelineStepId;
    *(newItem->mutable_producerinstanceid()) = producerInstanceId;
    *(newItem->mutable_beamtime()) = beamtime;
    *(newItem->mutable_source()) = source;
    *(newItem->mutable_stream()) = stream;

    return newItem;
}

RdsToConsumerDataPoint*
asapo::ReceiverMonitoringClientImpl::ToBeSendData::GetReceiverDataServerToConsumer(const std::string& pipelineStepId,
                                                                               const std::string& consumerInstanceId,
                                                                               const std::string& beamtime,
                                                                               const std::string& source,
                                                                               const std::string& stream) {
    for (auto& item : *(container.mutable_groupedrds2ctransfers())) {
        if (item.pipelinestepid() == pipelineStepId &&
        item.consumerinstanceid() == consumerInstanceId &&
        item.beamtime() == beamtime &&
        item.source() == source &&
        item.stream() == stream) {
            return &item;
        }
    }

    auto newItem = container.mutable_groupedrds2ctransfers()->Add();

    *(newItem->mutable_pipelinestepid()) = pipelineStepId;
    *(newItem->mutable_consumerinstanceid()) = consumerInstanceId;
    *(newItem->mutable_beamtime()) = beamtime;
    *(newItem->mutable_source()) = source;
    *(newItem->mutable_stream()) = stream;

    return newItem;
}

RdsMemoryDataPoint* asapo::ReceiverMonitoringClientImpl::ToBeSendData::GetMemoryDataPoint(const std::string& beamtime,
                                                                                      const std::string& source,
                                                                                      const std::string& stream) {
    for (auto& item : *(container.mutable_groupedmemorystats())) {
        if (item.beamtime() == beamtime &&
        item.source() == source &&
        item.stream() == stream) {
            return &item;
        }
    }

    auto newItem = container.mutable_groupedmemorystats()->Add();

    *(newItem->mutable_beamtime()) = beamtime;
    *(newItem->mutable_source()) = source;
    *(newItem->mutable_stream()) = stream;

    return newItem;
}

uint64_t asapo::ReceiverMonitoringClientImpl::WaitTimeMsUntilNextInterval(std::chrono::high_resolution_clock::time_point startTime) {
    auto now = std::chrono::high_resolution_clock::now();
    auto delta = now - startTime;
    auto interval = std::chrono::milliseconds(kUniversalSendingIntervalMs);

    auto result = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(interval - (delta % interval)).count();
    return result;
}

ReceiverMonitoringClientImpl::~ReceiverMonitoringClientImpl() {
    StopSendingThread();
}



