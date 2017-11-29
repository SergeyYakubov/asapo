#ifndef HIDRA2__PRODUCER_PRODUCERIMPL_H
#define HIDRA2__PRODUCER_PRODUCERIMPL_H

#include <string>
#include "producer/producer.h"

namespace HIDRA2
{
    class ProducerImpl : public Producer
    {
    private:
        static const uint32_t kVersion;
    public:
        ProducerImpl(const ProducerImpl&) = delete;
        ProducerImpl& operator=(const ProducerImpl&) = delete;
        ProducerImpl() = default;
        ~ProducerImpl() = default;

        uint64_t get_version() override;
        //ProducerError connect(std::string receiver_address) override;
    };
}

#endif //HIDRA2__PRODUCER_PRODUCERIMPL_H
