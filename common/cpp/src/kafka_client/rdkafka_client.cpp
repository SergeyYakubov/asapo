#include "rdkafka_client.h"

#include <cstring>
#include  "asapo/common/data_structs.h"
#include "asapo/io/io_factory.h"

namespace asapo {

RdKafkaClient::RdKafkaClient(const KafkaClientConfig& config) : default_topic_conf_(nullptr) {
    std::string err;
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    for (const auto& configItem : config.global_config) {
        if (conf->set(configItem.first, configItem.second, err) != RdKafka::Conf::CONF_OK) {
            throw "cannot initialize kafka: " + err;
        }
    }

    producer_ = RdKafka::Producer::create(conf, err);
    if (!producer_) {
        throw "cannot initialize kafka";
    }

    for (const auto& topic : config.topics_config) {
        auto topic_config = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
        for (const auto& configItem : topic.second) {
            if (topic_config->set(configItem.first, configItem.second, err) != RdKafka::Conf::CONF_OK) {
                throw "cannot initialize kafka: " + err;
            }
        }
        if (topic.first == "default") {
            default_topic_conf_ = topic_config;
        } else
        {
            auto topic_obj = RdKafka::Topic::create(producer_, topic.first, topic_config, err);
            if (!topic_obj) {
                throw "cannot initialize kafka topic [" + topic.first + "]: " + err;

            }
            kafka_topics_[topic.first] = topic_obj;
        }
    }
}

RdKafkaClient::~RdKafkaClient() {
    if (producer_) {
        producer_->flush(1000);
    }
    delete producer_;
}

Error RdKafkaClient::Send(const std::string& data, const std::string& topic_name) noexcept {
    auto topicIt = kafka_topics_.find(topic_name);
    RdKafka::Topic* topic;
    if (topicIt == kafka_topics_.end())
    {
        if (!default_topic_conf_) {
            return KafkaErrorTemplates::kUnknownTopicError.Generate();
        }
        std::string err;
        topic = RdKafka::Topic::create(producer_, topic_name, default_topic_conf_, err);
        if (!topic) {
            return KafkaErrorTemplates::kGeneralError.Generate("Cannot create kafka topic [" + topic_name + "]: " + err);
        }
        kafka_topics_[topic_name] = topic;
    }
    else
    {
        topic = topicIt->second;
    }

    auto err = producer_->produce(topic, RdKafka::Topic::PARTITION_UA,
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
