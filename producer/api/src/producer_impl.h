#ifndef HIDRA2_PRODUCER__PRODUCER_IMPL_H
#define HIDRA2_PRODUCER__PRODUCER_IMPL_H

#include <string>
#include <common/networking.h>
#include <system_wrappers/has_io.h>
#include "producer/producer.h"

namespace hidra2 {
class ProducerImpl : public Producer, public HasIO {
  private:
    static const uint32_t kVersion;

    int         client_fd_ = -1;
    uint64_t    request_id_ = 0;

    ProducerStatus status_ = ProducerStatus::kDisconnected;

    Error initialize_socket_to_receiver_(const std::string& receiver_address);
  public:
    static const size_t kMaxChunkSize;

    ProducerImpl();
    ProducerImpl(const ProducerImpl&) = delete;
    ProducerImpl& operator=(const ProducerImpl&) = delete;
    //~ProducerImpl() override;

    uint64_t GetVersion() const override;
    ProducerStatus GetStatus() const override;
    Error ConnectToReceiver(const std::string& receiver_address) override;
    Error Send(uint64_t file_id, const void* data, size_t file_size) override;

};
}

#endif //HIDRA2_PRODUCER__PRODUCER_IMPL_H
