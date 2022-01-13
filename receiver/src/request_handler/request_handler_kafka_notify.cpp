#include "request_handler_kafka_notify.h"
#include "../request.h"

namespace asapo {

Error RequestHandlerKafkaNotify::ProcessRequest(Request* request) const {
    bool online = request->GetSourceType() != SourceType::kProcessed &&
                  !static_cast<bool>(request->GetCustomData()[kPosIngestMode] & IngestModeFlags::kWriteRawDataToOffline);

    if (!kafka_client_ || !online) {
        //client was not initialized or file written to offline
        return nullptr;
    }

    auto root_folder = request->GetOnlinePath();
    if (root_folder.empty()) {
        return ReceiverErrorTemplates::kBadRequest.Generate("online path not available");
    }

    std::string message = "{"
            "\"event\":\"IN_CLOSE_WRITE\","
            "\"path\":\"" + root_folder + kPathSeparator + request->GetFileName() + "\""
    "}";

    return kafka_client_->Send(message, "asapo");
}

StatisticEntity RequestHandlerKafkaNotify::GetStatisticEntity() const {
    return StatisticEntity::kNetwork;
}

RequestHandlerKafkaNotify::RequestHandlerKafkaNotify(KafkaClient* kafka_client) : kafka_client_{kafka_client} {
}
}