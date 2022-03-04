#ifndef ASAPO_MOCKKAFKACLIENT_H
#define ASAPO_MOCKKAFKACLIENT_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/kafka_client/kafka_client.h"

namespace asapo {

class MockKafkaClient : public KafkaClient {
  public:
    Error Send(const std::string& data, const std::string& topic) noexcept override {
        return Error{Send_t(data, topic)};
    }

    MOCK_METHOD(ErrorInterface *, Send_t, (const std::string& data, const std::string& topic), ());
};

}

#endif //ASAPO_MOCKKAFKACLIENT_H
