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

class ProducerImpl : public Producer {
  private:
    // important to create it before request_pool__
    std::unique_ptr<ReceiverDiscoveryService> discovery_service_;
    std::unique_ptr<RequestHandlerFactory> request_handler_factory_;
  public:
    static const size_t kMaxChunkSize;
    static const size_t kDiscoveryServiceUpdateFrequencyMs;

    explicit ProducerImpl(std::string endpoint, uint8_t n_processing_threads, asapo::RequestHandlerType type);
    ProducerImpl(const ProducerImpl&) = delete;
    ProducerImpl& operator=(const ProducerImpl&) = delete;

    void SetLogLevel(LogLevel level) override;
    void EnableLocalLog(bool enable) override;
    void EnableRemoteLog(bool enable) override;
    Error SendData(const EventHeader& event_header, FileData data, std::string metadata, RequestCallback callback) override;
    Error SendFile(const EventHeader& event_header, std::string full_path, RequestCallback callback) override;
    AbstractLogger* log__;
    std::unique_ptr<RequestPool> request_pool__;
    Error SetBeamtimeId(std::string beamtime_id) override;
    Error SendMetaData(const std::string& metadata, RequestCallback callback) override;

  private:
    Error Send(const EventHeader& event_header, FileData data, std::string metadata, std::string full_path,
               RequestCallback callback);
    GenericRequestHeader GenerateNextSendRequest(const EventHeader& event_header, uint64_t meta_size);
    std::string beamtime_id_;
};

}

#endif //ASAPO_PRODUCER__PRODUCER_IMPL_H
