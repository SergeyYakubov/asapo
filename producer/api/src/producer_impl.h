#ifndef HIDRA2_PRODUCER__PRODUCER_IMPL_H
#define HIDRA2_PRODUCER__PRODUCER_IMPL_H

#include <string>
#include <system_wrappers/has_io.h>
#include "producer/producer.h"

namespace hidra2 {
class ProducerImpl : public Producer {
  private:
    static const uint32_t kVersion;

    int         client_fd_ = -1;
    uint64_t    request_id = 0;

    ProducerStatus status_ = PRODUCER_STATUS__DISCONNECTED;

    ProducerError initialize_socket_to_receiver_(const std::string& receiver_address);
  public:
    static const size_t kMaxChunkSize;

    ProducerImpl();
    ProducerImpl(const ProducerImpl&) = delete;
    ProducerImpl& operator=(const ProducerImpl&) = delete;
    //~ProducerImpl() override;

    uint64_t GetVersion() const override;
    ProducerStatus GetStatus() const override;
    ProducerError ConnectToReceiver(const std::string& receiver_address) override;
    ProducerError Send(std::string filename, void* data, uint64_t file_size) override;

};
}

#endif //HIDRA2_PRODUCER__PRODUCER_IMPL_H
