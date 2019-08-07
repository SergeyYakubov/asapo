#include "producer/producer.h"
#include "producer_impl.h"

std::unique_ptr<asapo::Producer> asapo::Producer::Create(const std::string& endpoint, uint8_t n_processing_threads,
        asapo::RequestHandlerType type, SourceCredentials source_cred, Error* err) {
    if (n_processing_threads > kMaxProcessingThreads) {
        *err = TextError("Too many processing threads: " + std::to_string(n_processing_threads));
        return nullptr;
    }

    std::unique_ptr<asapo::Producer> producer;
    try {
        producer.reset(new ProducerImpl(endpoint, n_processing_threads, type));
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
