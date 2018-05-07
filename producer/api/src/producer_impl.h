#ifndef ASAPO_PRODUCER__PRODUCER_IMPL_H
#define ASAPO_PRODUCER__PRODUCER_IMPL_H

#include <string>
#include <common/networking.h>
#include <io/io.h>
#include "producer/producer.h"
#include "logger/logger.h"

namespace asapo {
class ProducerImpl : public Producer {
  private:
    static const uint32_t kVersion;

    int         client_fd_ = -1;
    std::string receiver_uri_;
    uint64_t    request_id_ = 0;

    ProducerStatus status_ = ProducerStatus::kDisconnected;

    Error InitializeSocketToReceiver(const std::string& receiver_address);
    GenericNetworkRequestHeader GenerateNextSendRequest(uint64_t file_id, size_t file_size);
    Error SendHeaderAndData(const GenericNetworkRequestHeader& header, const void* data, size_t file_size);
    Error ReceiveResponce();

  public:
    static const size_t kMaxChunkSize;

    ProducerImpl();
    ProducerImpl(const ProducerImpl&) = delete;
    ProducerImpl& operator=(const ProducerImpl&) = delete;

    uint64_t GetVersion() const override;
    ProducerStatus GetStatus() const override;
    void SetLogLevel(LogLevel level) override;
    void EnableLocalLog(bool enable) override;
    void EnableRemoteLog(bool enable) override;
    Error ConnectToReceiver(const std::string& receiver_address) override;
    Error Send(uint64_t file_id, const void* data, size_t file_size) override;
    std::unique_ptr<IO> io__;
    Logger log__;
};
}

#endif //ASAPO_PRODUCER__PRODUCER_IMPL_H
