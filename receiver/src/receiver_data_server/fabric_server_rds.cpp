#include "fabric_server_rds.h"

using namespace asapo;

FabricServerRds::FabricServerRds(const std::string& address) {

}

FabricServerRds::~FabricServerRds() {

}

GenericRequests FabricServerRds::GetNewRequests(Error* err) const noexcept {
    return asapo::GenericRequests();
}

Error FabricServerRds::SendData(uint64_t source_id, void* buf, uint64_t size) const noexcept {
    return asapo::Error();
}

void FabricServerRds::HandleAfterError(uint64_t source_id) const noexcept {

}
