#include "producer/producer.h"
#include "producer_impl.h"


std::unique_ptr<HIDRA2::Producer> HIDRA2::Producer::create()
{
    return std::unique_ptr<HIDRA2::Producer>(new ProducerImpl());
}
