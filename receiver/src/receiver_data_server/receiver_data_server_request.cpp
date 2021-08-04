#include "receiver_data_server_request.h"

#include <utility>
#include "receiver_data_server.h"

namespace asapo {

ReceiverDataServerRequest::ReceiverDataServerRequest(const GenericRequestHeader& header, uint64_t source_id, SharedInstancedStatistics statistics) :
    GenericRequest(header, 0), statistics_{std::move(statistics)}, source_id{source_id} {
}

SharedInstancedStatistics ReceiverDataServerRequest::GetStatisticsProvider() {
    return statistics_;
}
}
