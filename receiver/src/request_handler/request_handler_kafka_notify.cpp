#include "request_handler_kafka_notify.h"
#include "../request.h"

namespace asapo {

Error RequestHandlerKafkaNotify::ProcessRequest(Request* request) const {
    if (!kafka_client_) {
        //client was not initialized, ignore
        return nullptr;
    }

    std::string message = "{"
            "\"event\":\"IN_CLOSE_WRITE\","
            "\"path\":\"" + request->GetFileName() + "\""
    "}";

    return kafka_client_->Send(message, "asapo");
}

StatisticEntity RequestHandlerKafkaNotify::GetStatisticEntity() const {
    return StatisticEntity::kNetwork;
}

RequestHandlerKafkaNotify::RequestHandlerKafkaNotify() : kafka_client_{GetKafkaClient()} {
}
}