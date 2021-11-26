#ifndef ASAPO_KAFKA_CONFIG_H
#define ASAPO_KAFKA_CONFIG_H

#include <string>
#include <map>

namespace asapo {

    struct KafkaClientConfig {
        std::map<std::string, std::string> global_config;
        std::map<std::string, std::map<std::string, std::string>> topics_config;
    };

}

#endif //ASAPO_KAFKA_CONFIG_H
