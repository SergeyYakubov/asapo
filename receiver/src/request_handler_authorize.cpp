#include "request_handler_authorize.h"
#include "receiver_config.h"
#include "receiver_logger.h"

namespace asapo {

Error RequestHandlerAuthorize::ProcessRequest(const Request& request) const {
    return nullptr;
}

RequestHandlerAuthorize::RequestHandlerAuthorize(): log__{GetDefaultReceiverLogger()}  {
}

StatisticEntity RequestHandlerAuthorize::GetStatisticEntity() const {
    return StatisticEntity::kDatabase;
}


}