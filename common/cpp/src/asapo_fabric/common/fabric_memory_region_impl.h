#ifndef ASAPO_FABRIC_MEMORY_REGION_IMPL_H
#define ASAPO_FABRIC_MEMORY_REGION_IMPL_H

#include <asapo_fabric/asapo_fabric.h>
#include <rdma/fi_domain.h>

namespace asapo {
namespace fabric {
class FabricMemoryRegionImpl : public FabricMemoryRegion {
  private:
    fid_mr* mr_{};
    MemoryRegionDetails details_{};
  public:
    ~FabricMemoryRegionImpl() override;

    void SetArguments(fid_mr* mr, uint64_t address, uint64_t length);

    const MemoryRegionDetails* GetDetails() const override;
};
}
}


#endif //ASAPO_FABRIC_MEMORY_REGION_IMPL_H
