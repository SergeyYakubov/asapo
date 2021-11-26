#ifndef ASAPO_RDKAFKA_CLIENT_H
#define ASAPO_RDKAFKA_CLIENT_H

#include <string>

#include "asapo/kafka_client/kafka_client.h"
#include "librdkafka/rdkafkacpp.h"

namespace asapo {

class RdKafkaClient final : public KafkaClient {
  public:
    RdKafkaClient(const KafkaClientConfig& config);
    Error Send(const std::string& data, const std::string& topic) noexcept override;

    virtual ~RdKafkaClient();
  private:
    RdKafka::Producer* producer;
    RdKafka::Conf* defaultTopicConf;
    std::map<std::string, RdKafka::Topic *> kafkaTopics;
};

}

#endif //ASAPO_RDKAFKA_CLIENT_H
