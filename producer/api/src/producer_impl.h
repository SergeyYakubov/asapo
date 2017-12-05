#ifndef HIDRA2_PRODUCER__PRODUCERIMPL_H
#define HIDRA2_PRODUCER__PRODUCERIMPL_H

#include <string>
#include <system_wrappers/has_io.h>
#include "producer/producer.h"

namespace hidra2 {
class ProducerImpl : public Producer {
    friend Producer;
  private:
    static const uint32_t kVersion;
    static FileReferenceId kGlobalReferenceId;
  public:
    ProducerImpl();
    ProducerImpl(const ProducerImpl &) = delete;
    ProducerImpl &operator=(const ProducerImpl &) = delete;
    //~ProducerImpl() override;

    uint64_t get_version() const override;
    ProducerStatus get_status() const override;
    ProducerError connect_to_receiver(std::string receiver_address) override;
    ProducerError send(std::string filename, uint64_t file_size, void* data) override;

};
}

#endif //HIDRA2_PRODUCER__PRODUCERIMPL_H
