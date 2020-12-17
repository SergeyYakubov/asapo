#include "asapo/producer/producer.h"
#include "producer_impl.h"
#include "asapo/producer/producer_error.h"

std::unique_ptr<asapo::Producer> asapo::Producer::Create(const std::string& endpoint, uint8_t n_processing_threads,
        asapo::RequestHandlerType type, SourceCredentials source_cred, uint64_t timeout_sec, Error* err) {

    if (n_processing_threads > kMaxProcessingThreads || n_processing_threads == 0) {
        *err = ProducerErrorTemplates::kWrongInput.Generate("Set number of processing threads > 0 and <= " + std::to_string(
                    kMaxProcessingThreads));
        return nullptr;
    }

    std::unique_ptr<asapo::Producer> producer;
    try {
        producer.reset(new ProducerImpl(endpoint, n_processing_threads, timeout_sec, type));
    } catch (const std::exception& ex) {
        *err = TextError(ex.what());
        return nullptr;
    } catch (...) {
        *err = TextError("Unknown exception in producer_api ");
        return nullptr;
    }

    *err = producer->SetCredentials(std::move(source_cred));
    if (*err) {
        return nullptr;
    }
    return producer;
}
