#include <producer/producer.h>

unsigned long hidra2::Producer::kinit_count_ = 0;
const uint32_t hidra2::Producer::VERSION = 1;

hidra2::Producer::Producer() {
    kinit_count_++;
}

hidra2::Producer* hidra2::Producer::CreateProducer(std::string receiver_address) {
    return new Producer();
}
