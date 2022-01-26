#include "receiver_data_server_request.h"

#include <utility>
#include "receiver_data_server.h"

namespace asapo {

ReceiverDataServerRequest::ReceiverDataServerRequest(const GenericRequestHeader& header, uint64_t source_id, RequestStatisticsPtr statistics) :
    GenericRequest(header, 0), statistics_{std::move(statistics)}, source_id{source_id} {
}

RequestStatistics* ReceiverDataServerRequest::GetStatistics() {
    return statistics_.get();
}
}
