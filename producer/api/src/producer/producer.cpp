#include <producer/producer.h>

unsigned long HIDRA2::Producer::kInitCount = 0;
const uint32_t HIDRA2::Producer::kVersion = 1;

HIDRA2::Producer::Producer()
{
    kInitCount++;
}

HIDRA2::Producer *HIDRA2::Producer::CreateProducer(std::string receiver_address)
{
    return new Producer();
}
