#ifndef ASAPO_PRODUCER__PRODUCER_IMPL_H
#define ASAPO_PRODUCER__PRODUCER_IMPL_H

#include <string>
#include <asapo/common/networking.h>
#include <asapo/io/io.h>
#include "asapo/producer/producer.h"
#include "asapo/logger/logger.h"
#include "asapo/request/request_pool.h"
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

    explicit ProducerImpl(std::string endpoint, uint8_t n_processing_threads, uint64_t timeout_ms,
                          asapo::RequestHandlerType type);
    ProducerImpl(const ProducerImpl&) = delete;
    ProducerImpl& operator=(const ProducerImpl&) = delete;


    Error GetVersionInfo(std::string* client_info, std::string* server_info, bool* supported) const override;

    StreamInfo GetStreamInfo(std::string stream, uint64_t timeout_ms, Error* err) const override;
    StreamInfo GetLastStream(uint64_t timeout_ms, Error* err) const override;

    void SetLogLevel(LogLevel level) override;
    void EnableLocalLog(bool enable) override;
    void EnableRemoteLog(bool enable) override;
    Error Send(const MessageHeader& message_header,
               MessageData data,
               uint64_t ingest_mode,
               std::string stream,
               RequestCallback callback) override;
    Error Send__(const MessageHeader& message_header,
                 void* data,
                 uint64_t ingest_mode,
                 std::string stream,
                 RequestCallback callback) override;
    void StopThreads__() override;
    Error SendFile(const MessageHeader& message_header,
                   std::string full_path,
                   uint64_t ingest_mode,
                   std::string stream,
                   RequestCallback callback) override;
    Error SendStreamFinishedFlag(std::string stream, uint64_t last_id, std::string next_stream,
                                 RequestCallback callback) override;

    Error DeleteStream(std::string stream, uint64_t timeout_ms, DeleteStreamOptions options) const override;

    AbstractLogger* log__;
    std::unique_ptr<HttpClient> httpclient__;
    std::unique_ptr<RequestPool> request_pool__;

    Error SetCredentials(SourceCredentials source_cred) override;

    Error SendMetadata(const std::string& metadata, RequestCallback callback) override;
    Error SendBeamtimeMetadata(const std::string& metadata, MetaIngestMode mode, RequestCallback callback) override;
    Error SendStreamMetadata(const std::string& stream, const std::string& metadata, MetaIngestMode mode, RequestCallback callback) override;

  uint64_t GetRequestsQueueSize() override;
    Error WaitRequestsFinished(uint64_t timeout_ms) override;
    uint64_t GetRequestsQueueVolumeMb() override;
    void SetRequestsQueueLimits(uint64_t size, uint64_t volume) override;
  private:
    Error SendMeta(const std::string stream, const std::string &metadata, MetaIngestMode mode, RequestCallback callback);
    StreamInfo StreamRequest(StreamRequestOp op, std::string stream, uint64_t timeout_ms, Error* err) const;
    Error Send(const MessageHeader& message_header, std::string stream, MessageData data, std::string full_path,
               uint64_t ingest_mode,
               RequestCallback callback, bool manage_data_memory);
    GenericRequestHeader GenerateNextSendRequest(const MessageHeader& message_header, std::string stream,
                                                 uint64_t ingest_mode);
    std::string source_cred_string_;
    uint64_t timeout_ms_;
    std::string endpoint_;
    Error GetServerVersionInfo(std::string* server_info,
                               bool* supported) const;
};

struct StreamInfoResult {
    StreamInfo sinfo;
    ErrorInterface* err;
};

}

#endif //ASAPO_PRODUCER__PRODUCER_IMPL_H
