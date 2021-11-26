#include "rdkafka_client.h"

#include <cstring>
#include  "asapo/common/data_structs.h"
#include "asapo/io/io_factory.h"

namespace asapo {

RdKafkaClient::RdKafkaClient(const KafkaClientConfig& config) : defaultTopicConf(nullptr) {
    std::string err;
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    for (const auto& configItem : config.global_config) {
        if (conf->set(configItem.first, configItem.second, err) != RdKafka::Conf::CONF_OK) {
            throw "cannot initialize kafka: " + err;
        }
    }

    producer = RdKafka::Producer::create(conf, err);
    if (!producer) {
        throw "cannot initialize kafka";
    }

    for (const auto& topic : config.topics_config) {
        auto topicConfig = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
        for (const auto& configItem : topic.second) {
            if (topicConfig->set(configItem.first, configItem.second, err) != RdKafka::Conf::CONF_OK) {
                throw "cannot initialize kafka: " + err;
            }
        }
        if (topic.first == "default") {
            this->defaultTopicConf = topicConfig;
        } else
        {
            auto topicObj = RdKafka::Topic::create(producer, topic.first, topicConfig, err);
            if (!topicObj) {
                throw "cannot initialize kafka topic [" + topic.first + "]: " + err;

            }
            this->kafkaTopics[topic.first] = topicObj;
        }
    }
}

RdKafkaClient::~RdKafkaClient() {
    if (producer) {
        producer->flush(1000);
    }
    delete producer;
}

Error RdKafkaClient::Send(const std::string& data, const std::string& topicName) noexcept {
    auto topicIt = this->kafkaTopics.find(topicName);
    RdKafka::Topic* topic;
    if (topicIt == this->kafkaTopics.end())
    {
        if (!defaultTopicConf) {
            return KafkaErrorTemplates::kUnknownTopicError.Generate();
        }
        std::string err;
        topic = RdKafka::Topic::create(producer, topicName, this->defaultTopicConf, err);
        if (!topic) {
            return KafkaErrorTemplates::kGeneralError.Generate("Cannot create kafka topic [" + topicName + "]: " + err);
        }
        this->kafkaTopics[topicName] = topic;
    }
    else
    {
        topic = topicIt->second;
    }

    auto err = producer->produce(topic, RdKafka::Topic::PARTITION_UA,
                                 RdKafka::Producer::RK_MSG_COPY,
                                 const_cast<void*>(static_cast<const void *>(data.data())), data.size(),
                                 nullptr, nullptr);

    switch (err) {
        case RdKafka::ERR_NO_ERROR: return nullptr;
        case RdKafka::ERR__QUEUE_FULL: return KafkaErrorTemplates::kQueueFullError.Generate();
        case RdKafka::ERR_MSG_SIZE_TOO_LARGE: return KafkaErrorTemplates::kMessageTooLargeError.Generate();
        case RdKafka::ERR__UNKNOWN_PARTITION: return KafkaErrorTemplates::kUnknownPartitionError.Generate();
        case RdKafka::ERR__UNKNOWN_TOPIC: return KafkaErrorTemplates::kUnknownTopicError.Generate();
        default: return KafkaErrorTemplates::kGeneralError.Generate(err2str(err));
    }

}

}
