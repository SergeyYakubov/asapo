#include "producer/producer.h"
#include "producer_impl.h"

std::unique_ptr<asapo::Producer> asapo::Producer::Create() {
    return std::unique_ptr<asapo::Producer>(new ProducerImpl());
}
