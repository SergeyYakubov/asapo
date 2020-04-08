#include "fabric_server_rds.h"

using namespace asapo;

FabricServerRds::FabricServerRds(const std::string& address) : factory__(fabric::GenerateDefaultFabricFactory()) {

}

FabricServerRds::~FabricServerRds() {

}

GenericRequests FabricServerRds::GetNewRequests(Error* err) const noexcept {
    return asapo::GenericRequests();
}

Error
FabricServerRds::SendResponse(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response) const noexcept {
    return asapo::Error();
}

Error FabricServerRds::SendResponseAndSlotData(const ReceiverDataServerRequest* request,
                                               const GenericNetworkResponse* response,
                                               const CacheMeta* cache_slot) const noexcept {
    return asapo::Error();
}

void FabricServerRds::HandleAfterError(uint64_t source_id) const noexcept {

}
