#include "rdkafka_client.h"

namespace asapo {

KafkaClient* CreateKafkaClient(const KafkaClientConfig& config, Error* err) {
    try {
        return new RdKafkaClient(config);
    }
    catch (std::string errstr) {
        (*err) = KafkaErrorTemplates::kGeneralError.Generate(errstr);
    }
    return nullptr;
}
}
