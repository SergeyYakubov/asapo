#ifndef ASAPO_FABRIC_ALIVE_CHECK_RESPONSE_TASK_H
#define ASAPO_FABRIC_ALIVE_CHECK_RESPONSE_TASK_H

#include "fabric_self_requeuing_task.h"

namespace asapo {
namespace fabric {

/**
 * This is the counter part of FabricContextImpl.TargetIsAliveCheck
 */
class FabricAliveCheckResponseTask : public FabricSelfRequeuingTask {
  public:
    explicit FabricAliveCheckResponseTask(FabricContextImpl* parentContext);
  protected:
    void RequeueSelf(FabricContextImpl* parentContext) override;

    void OnCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) override;

    void OnErrorCompletion(const fi_cq_err_entry* errEntry) override;
};
}
}

#endif //ASAPO_FABRIC_ALIVE_CHECK_RESPONSE_TASK_H
