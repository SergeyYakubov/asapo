#include "fabric_rds_request.h"

using namespace asapo;

FabricRdsRequest::FabricRdsRequest( GenericRequestHeader header,
                                    fabric::FabricAddress sourceId, fabric::FabricMessageId messageId)
        : ReceiverDataServerRequest(header, sourceId), message_id{messageId} {

}

const fabric::MemoryRegionDetails* asapo::FabricRdsRequest::GetMemoryRegion() const {
    return reinterpret_cast<const fabric::MemoryRegionDetails*>(header.message);
}

