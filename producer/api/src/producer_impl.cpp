#include "producer_impl.h"

const uint32_t HIDRA2::ProducerImpl::kVersion = 1;
HIDRA2::FileReferenceId HIDRA2::ProducerImpl::kGlobalReferenceId = 0;


uint64_t HIDRA2::ProducerImpl::get_version() const
{
    return kVersion;
}

HIDRA2::ProducerStatus HIDRA2::ProducerImpl::get_status() const
{
    return PRODUCER_STATUS__CONNECTED;
}

HIDRA2::ProducerError HIDRA2::ProducerImpl::connect(std::string receiver_address)
{
    return PRODUCER_ERROR__OK;
}

HIDRA2::FileReferenceId
HIDRA2::ProducerImpl::send(std::string filename, uint64_t file_size, Yieldable<HIDRA2::FileChunk>* chunk_provider,
                           std::function<void(HIDRA2::FileChunk)> after_processing,
                           std::function<void(HIDRA2::FileReferenceId, HIDRA2::ProducerError)> file_done,
                           HIDRA2::ProducerError &error)
{
    HIDRA2::FileReferenceId ref_id = kGlobalReferenceId++;

    if(chunk_provider->is_done()) {
        return PRODUCER_ERROR__CHUNK_PROVIDER_NOT_READY_AT_START;
    }
    do {
        chunk_provider->next();
    } while(!chunk_provider->is_done());

    file_done(ref_id, HIDRA2::ProducerError::PRODUCER_ERROR__OK);

    error = PRODUCER_ERROR__OK;
    return ref_id;
}
/*
HIDRA2::ProducerImpl::~ProducerImpl()
{

}
 */


