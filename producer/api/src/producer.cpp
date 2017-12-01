#include "producer/producer.h"
#include "producer_impl.h"


std::unique_ptr<hidra2::Producer> hidra2::Producer::create()
{
    return std::unique_ptr<hidra2::Producer>(new ProducerImpl());
}
