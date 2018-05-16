#include "producer/producer.h"
#include "producer_impl.h"

std::unique_ptr<asapo::Producer> asapo::Producer::Create(const std::string& endpoint, uint8_t n_processing_threads,
        Error* err) {
    if (n_processing_threads > kMaxProcessingThreads) {
        *err = TextError("Too many processing threads: " + std::to_string(n_processing_threads));
        return nullptr;
    }

    try {
        *err = nullptr;
        return std::unique_ptr<asapo::Producer>(new ProducerImpl(endpoint, n_processing_threads));
    } catch (const std::exception& ex) {
        *err = TextError(ex.what());
        return nullptr;
    } catch (...) {
        *err = TextError("Unknown exception in producer_api ");
        return nullptr;
    }
}
