#include "fabric_memory_region_impl.h"

using namespace asapo;
using namespace fabric;

FabricMemoryRegionImpl::~FabricMemoryRegionImpl() {
    if (mr_) {
        fi_close(&mr_->fid);
    }
}

void FabricMemoryRegionImpl::SetArguments(fid_mr* mr, uint64_t address, uint64_t length) {
    mr_ = mr;
    details_.addr = address;
    details_.length = length;
    details_.key = fi_mr_key(mr_);
}

const MemoryRegionDetails* FabricMemoryRegionImpl::GetDetails() const {
    return &details_;
}
