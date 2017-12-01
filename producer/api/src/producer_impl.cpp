#include "producer_impl.h"

const uint32_t hidra2::ProducerImpl::kVersion = 1;
hidra2::FileReferenceId hidra2::ProducerImpl::kGlobalReferenceId = 0;

uint64_t hidra2::ProducerImpl::get_version() const {
    return kVersion;
}

hidra2::ProducerStatus hidra2::ProducerImpl::get_status() const {
    return PRODUCER_STATUS__CONNECTED;
}

hidra2::ProducerError hidra2::ProducerImpl::connectToReceiver(std::string receiver_address) {
    return PRODUCER_ERROR__OK;
}

hidra2::FileReferenceId
hidra2::ProducerImpl::send(std::string filename, uint64_t file_size, Yieldable<hidra2::FileChunk>* chunk_provider,
                           std::function<void(hidra2::FileChunk)> after_processing,
                           std::function<void(hidra2::FileReferenceId, hidra2::ProducerError)> file_done,
                           hidra2::ProducerError &error) {
    hidra2::FileReferenceId ref_id = kGlobalReferenceId++;

    if (chunk_provider->is_done()) {
        return PRODUCER_ERROR__CHUNK_PROVIDER_NOT_READY_AT_START;
    }
    do {
        chunk_provider->next();
    } while (!chunk_provider->is_done());

    file_done(ref_id, hidra2::ProducerError::PRODUCER_ERROR__OK);

    error = PRODUCER_ERROR__OK;
    return ref_id;
}
/*
hidra2::ProducerImpl::~ProducerImpl()
{

}
 */


