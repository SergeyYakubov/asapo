#ifndef ASAPO_FABRIC_RDS_REQUEST_H
#define ASAPO_FABRIC_RDS_REQUEST_H

#include <asapo/asapo_fabric/asapo_fabric.h>
#include "../receiver_data_server_request.h"

namespace asapo {

class FabricRdsRequest : public ReceiverDataServerRequest {
  public:
    explicit FabricRdsRequest(const GenericRequestHeader& header, fabric::FabricAddress source_id,
                              fabric::FabricMessageId messageId, SharedInstancedStatistics statistics);
    fabric::FabricMessageId message_id;
    const fabric::MemoryRegionDetails* GetMemoryRegion() const;
};

}

#endif //ASAPO_FABRIC_RDS_REQUEST_H
