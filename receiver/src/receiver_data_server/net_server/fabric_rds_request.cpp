#include "fabric_rds_request.h"

#include <utility>

using namespace asapo;

FabricRdsRequest::FabricRdsRequest(const GenericRequestHeader& header,
                                   fabric::FabricAddress sourceId, fabric::FabricMessageId messageId, RequestStatisticsPtr statistics)
                                   : ReceiverDataServerRequest(header, sourceId, std::move(statistics)), message_id{messageId} {

}

const fabric::MemoryRegionDetails* asapo::FabricRdsRequest::GetMemoryRegion() const {
    return reinterpret_cast<const fabric::MemoryRegionDetails*>(header.message);
}

