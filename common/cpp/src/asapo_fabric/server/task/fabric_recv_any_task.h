#ifndef ASAPO_FABRIC_RECV_ANY_TASK_H
#define ASAPO_FABRIC_RECV_ANY_TASK_H

#include <asapo/asapo_fabric/asapo_fabric.h>
#include <rdma/fi_eq.h>
#include "../../common/task/fabric_waitable_task.h"

namespace asapo {
namespace fabric {

class FabricRecvAnyTask : public FabricWaitableTask {
  private:
    FabricMessageId messageId_;
  public:
    void HandleCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) override;
    void HandleErrorCompletion(const fi_cq_err_entry* errEntry) override;

    FabricMessageId GetMessageId() const;
};

}
}


#endif //ASAPO_FABRIC_RECV_ANY_TASK_H
