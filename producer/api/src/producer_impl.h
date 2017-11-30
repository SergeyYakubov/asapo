#ifndef HIDRA2_PRODUCER__PRODUCERIMPL_H
#define HIDRA2_PRODUCER__PRODUCERIMPL_H

#include <string>
#include "producer/producer.h"

namespace HIDRA2
{
    class ProducerImpl : public Producer
    {
    private:
        static const uint32_t kVersion;
        static FileReferenceId kGlobalReferenceId;

    public:
        ProducerImpl(const ProducerImpl&) = delete;
        ProducerImpl& operator=(const ProducerImpl&) = delete;
        ProducerImpl() = default;
        //~ProducerImpl() override;

        uint64_t get_version() const override;
        ProducerStatus get_status() const override;
        ProducerError connect(std::string receiver_address) override;
        FileReferenceId send(std::string filename,
                             uint64_t file_size,
                             Yieldable<FileChunk>* chunk_provider,
                             std::function<void(FileChunk)> after_processing,
                             std::function<void(FileReferenceId, ProducerError)> file_done,
                             ProducerError& error) override;
    };
}

#endif //HIDRA2_PRODUCER__PRODUCERIMPL_H
