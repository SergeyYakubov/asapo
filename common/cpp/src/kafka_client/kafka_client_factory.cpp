#include "rdkafka_client.h"

namespace asapo {

KafkaClient* instance = nullptr;

Error InitializeKafkaClient(const KafkaClientConfig& config) {
    if (instance != nullptr) {
        return KafkaErrorTemplates::kGeneralError.Generate("Kafka client already initialized");
    }

    try {
        instance = new RdKafkaClient(config);
    }
    catch (std::string err) {
        return KafkaErrorTemplates::kGeneralError.Generate(err);
    }

    return nullptr;
}

std::unique_ptr<KafkaClient> GetKafkaClient() {
    if (instance != nullptr) {
        return std::unique_ptr<KafkaClient> {instance};
    } else {
        return nullptr;
    }
}
}
