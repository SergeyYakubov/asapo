#ifndef ASAPO_PRODUCER__PRODUCER_IMPL_H
#define ASAPO_PRODUCER__PRODUCER_IMPL_H

#include <string>
#include <common/networking.h>
#include <io/io.h>
#include "producer/producer.h"
#include "logger/logger.h"
#include "request/request_pool.h"
#include "producer_request_handler_factory.h"
#include "receiver_discovery_service.h"

namespace asapo {

enum class StreamRequestOp {
  kStreamInfo,
  kLastStream
};

class ProducerImpl : public Producer {
  private:
    // important to create it before request_pool__
    std::unique_ptr<ReceiverDiscoveryService> discovery_service_;
    std::unique_ptr<RequestHandlerFactory> request_handler_factory_;
  public:
    static const size_t kDiscoveryServiceUpdateFrequencyMs;
    static const std::string kFinishSubStreamKeyword;
    static const std::string kNoNextSubStreamKeyword;

    explicit ProducerImpl(std::string endpoint, uint8_t n_processing_threads, uint64_t timeout_sec,
                          asapo::RequestHandlerType type);
    ProducerImpl(const ProducerImpl&) = delete;
    ProducerImpl& operator=(const ProducerImpl&) = delete;

    StreamInfo GetStreamInfo(std::string substream, uint64_t timeout_sec, Error* err) const override ;
    StreamInfo GetStreamInfo(uint64_t timeout_sec, Error* err) const override;
    StreamInfo GetLastSubstream(uint64_t timeout_sec, Error* err) const override;

    void SetLogLevel(LogLevel level) override;
    void EnableLocalLog(bool enable) override;
    void EnableRemoteLog(bool enable) override;
    Error SendData(const EventHeader& event_header, FileData data, uint64_t ingest_mode, RequestCallback callback) override;
    Error SendData__(const EventHeader& event_header, void* data, uint64_t ingest_mode,
                     RequestCallback callback) override;
    Error SendData(const EventHeader& event_header, std::string substream, FileData data, uint64_t ingest_mode,
                   RequestCallback callback) override;
    Error SendData__(const EventHeader& event_header, std::string substream, void* data, uint64_t ingest_mode,
                     RequestCallback callback) override;
    void StopThreads__() override;
    Error SendFile(const EventHeader& event_header, std::string full_path, uint64_t ingest_mode,
                   RequestCallback callback) override;
    Error SendFile(const EventHeader& event_header, std::string substream, std::string full_path, uint64_t ingest_mode,
                   RequestCallback callback) override;

    Error SendSubstreamFinishedFlag(std::string substream, uint64_t last_id, std::string next_substream,
                                    RequestCallback callback) override;


    AbstractLogger* log__;
    std::unique_ptr<RequestPool> request_pool__;

    Error SetCredentials(SourceCredentials source_cred) override;

    Error SendMetaData(const std::string& metadata, RequestCallback callback) override;
    uint64_t  GetRequestsQueueSize() override;
    Error WaitRequestsFinished(uint64_t timeout_ms) override;

  private:
    StreamInfo StreamRequest(StreamRequestOp op, std::string substream, uint64_t timeout_sec, Error* err) const;
    Error Send(const EventHeader& event_header, std::string substream, FileData data, std::string full_path,
               uint64_t ingest_mode,
               RequestCallback callback, bool manage_data_memory);
    GenericRequestHeader GenerateNextSendRequest(const EventHeader& event_header, std::string substream,
                                                 uint64_t ingest_mode);
    std::string source_cred_string_;
    uint64_t timeout_sec_;
};

struct StreamInfoResult {
    StreamInfo sinfo;
    ErrorInterface* err;
};




}

#endif //ASAPO_PRODUCER__PRODUCER_IMPL_H
