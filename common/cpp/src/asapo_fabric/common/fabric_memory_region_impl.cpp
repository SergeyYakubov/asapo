#include "fabric_memory_region_impl.h"

asapo::fabric::FabricMemoryRegionImpl::~FabricMemoryRegionImpl() {
    if (mr_) {
        fi_close(&mr_->fid);
    }
}

void asapo::fabric::FabricMemoryRegionImpl::SetArguments(fid_mr* mr, uint64_t address, uint64_t length) {
    mr_ = mr;
    details_.addr = address;
    details_.length = length;
    details_.key = fi_mr_key(mr_);
}

asapo::fabric::MemoryRegionDetails* asapo::fabric::FabricMemoryRegionImpl::GetDetails() {
    return &details_;
}
