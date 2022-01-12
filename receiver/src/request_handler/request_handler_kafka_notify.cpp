#include "request_handler_kafka_notify.h"
#include "../request.h"
#include "file_processors/file_processor.h"

namespace asapo {

Error RequestHandlerKafkaNotify::ProcessRequest(Request* request) const {
    if (!kafka_client_) {
        //client was not initialized, ignore
        return nullptr;
    }

    std::string root_folder;

    if (auto err = GetRootFolder(request, &root_folder)){
        return err;
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