#ifndef ASAPO_KAFKA_ERROR_H
#define ASAPO_KAFKA_ERROR_H

#include "asapo/common/error.h"

namespace asapo {

    enum class KafkaErrorType {
        kQueueFullError,
        kMessageTooLargeError,
        kUnknownPartitionError,
        kUnknownTopicError,
        kGeneralError
    };

    using KafkaErrorTemplate = ServiceErrorTemplate<KafkaErrorType>;

    namespace KafkaErrorTemplates {

        auto const kQueueFullError = KafkaErrorTemplate {
                "kafka queue is full", KafkaErrorType::kQueueFullError
        };

        auto const kMessageTooLargeError = KafkaErrorTemplate {
                "kafka message is too large", KafkaErrorType::kMessageTooLargeError
        };

        auto const kUnknownPartitionError = KafkaErrorTemplate {
                "partition is unknown in the kafka cluster", KafkaErrorType::kUnknownPartitionError
        };

        auto const kUnknownTopicError = KafkaErrorTemplate {
                "partition is unknown in the kafka cluster", KafkaErrorType::kUnknownTopicError
        };

        auto const kGeneralError = KafkaErrorTemplate {
                "unexpected kafka error occurred", KafkaErrorType::kGeneralError
        };
    }

}

#endif //ASAPO_KAFKA_ERROR_H
