#ifndef ASAPO_FABRIC_SELF_DELETING_TASK_H
#define ASAPO_FABRIC_SELF_DELETING_TASK_H

#include "../../common/fabric_task.h"

namespace asapo { namespace fabric {

class FabricSelfDeletingTask : FabricTask {

    void HandleCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) final;
    void HandleErrorCompletion(fi_cq_err_entry* errEntry) final;

private:
    virtual ~FabricSelfDeletingTask() = default;
    void OnDone();
};

}}

#endif //ASAPO_FABRIC_SELF_DELETING_TASK_H
