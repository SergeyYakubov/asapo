#include "producer/producer.h"
#include "producer_impl.h"


HIDRA2::Producer* HIDRA2::Producer::create()
{
    return new ProducerImpl();
}
