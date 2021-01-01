#ifndef ASAPO_FABRIC_TASK_H
#define ASAPO_FABRIC_TASK_H

#include <asapo/asapo_fabric/asapo_fabric.h>
#include <rdma/fi_eq.h>

namespace asapo {
namespace fabric {
class FabricTask {
  public:
    virtual void HandleCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) = 0;
    virtual void HandleErrorCompletion(const fi_cq_err_entry* errEntry) = 0;
};
}
}

#endif //ASAPO_FABRIC_TASK_H
