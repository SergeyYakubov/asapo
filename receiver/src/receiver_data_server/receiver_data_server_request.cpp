#include "receiver_data_server_request.h"
#include "receiver_data_server.h"

namespace asapo {

ReceiverDataServerRequest::ReceiverDataServerRequest(GenericRequestHeader header, uint64_t source_id) :
    GenericRequest(std::move(header)),
    source_id{source_id} {
}



}