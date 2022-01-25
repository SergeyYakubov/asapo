#ifndef ASAPO_REQUEST_HANDLER_KAFKA_NOTIFY_H
#define ASAPO_REQUEST_HANDLER_KAFKA_NOTIFY_H

#include "request_handler.h"
#include "asapo/kafka_client/kafka_client.h"

namespace asapo {

class RequestHandlerKafkaNotify final : public ReceiverRequestHandler {
  public:
    RequestHandlerKafkaNotify(KafkaClient* kafka_client);
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
  private:
    KafkaClient* kafka_client_;
};

}

#endif //ASAPO_REQUEST_HANDLER_KAFKA_NOTIFY_H
