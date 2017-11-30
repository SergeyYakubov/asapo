#include <producer/producer.h>

unsigned long HIDRA2::Producer::kinit_count_ = 0;
const uint32_t HIDRA2::Producer::VERSION = 1;

HIDRA2::Producer::Producer() {
  kinit_count_++;
}

HIDRA2::Producer *HIDRA2::Producer::CreateProducer(std::string receiver_address) {
  return new Producer();
}
