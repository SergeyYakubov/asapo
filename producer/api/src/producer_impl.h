#ifndef ASAPO_PRODUCER__PRODUCER_IMPL_H
#define ASAPO_PRODUCER__PRODUCER_IMPL_H

#include <string>
#include <common/networking.h>
#include <io/io.h>
#include "producer/producer.h"
#include "logger/logger.h"
#include "request_pool.h"
#include "request_handler_factory.h"
#include "receiver_discovery_service.h"

namespace asapo {

class ProducerImpl : public Producer {
  private:
    // important to create it before request_pool__
    std::unique_ptr<ReceiverDiscoveryService> discovery_service_;
    std::unique_ptr<RequestHandlerFactory> request_handler_factory_;
  public:
    static const size_t kMaxChunkSize;
    static const size_t kMaxPoolVolume;
    static const size_t kDiscoveryServiceUpdateFrequencyMs;

    explicit ProducerImpl(std::string endpoint, uint8_t n_processing_threads);
    ProducerImpl(const ProducerImpl&) = delete;
    ProducerImpl& operator=(const ProducerImpl&) = delete;

    void SetLogLevel(LogLevel level) override;
    void EnableLocalLog(bool enable) override;
    void EnableRemoteLog(bool enable) override;
    Error Send(uint64_t file_id, const void* data, size_t file_size, RequestCallback callback) override;
    AbstractLogger* log__;
    std::unique_ptr<RequestPool> request_pool__;
  private:
    GenericNetworkRequestHeader GenerateNextSendRequest(uint64_t file_id, size_t file_size);
};

Error CheckProducerRequest(const GenericNetworkRequestHeader header);
}

#endif //ASAPO_PRODUCER__PRODUCER_IMPL_H
