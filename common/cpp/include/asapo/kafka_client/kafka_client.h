#ifndef ASAPO_KAFKA_CLIENT_H
#define ASAPO_KAFKA_CLIENT_H

#include "asapo/common/error.h"
#include "asapo/common/data_structs.h"
#include "asapo/kafka_client/kafka_config.h"
#include "asapo/kafka_client/kafka_error.h"
#include <map>

namespace asapo {

class KafkaClient {
  public:
    virtual Error Send(const std::string& data,
                       const std::string& topic) noexcept = 0;
    virtual ~KafkaClient() = default;
};

KafkaClient* CreateKafkaClient(const KafkaClientConfig& config, Error* err);

}

#endif //ASAPO_KAFKA_CLIENT_H
