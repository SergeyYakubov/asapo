#ifndef ASAPO_FABRIC_WAITABLE_TASK_H
#define ASAPO_FABRIC_WAITABLE_TASK_H

#include <common/error.h>
#include <asapo_fabric/asapo_fabric.h>
#include <future>
#include "fabric_task.h"

namespace asapo {
namespace fabric {
class FabricWaitableTask : FabricTask {
  private:
    std::promise<void> promise_;
    std::future<void> future_;

    Error error_;
    FabricAddress source_;
  public:
    explicit FabricWaitableTask();

    void HandleCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) override;
    void HandleErrorCompletion(fi_cq_err_entry* errEntry) override;

    void Wait(uint32_t sleepInMs, Error* error);

    FabricAddress GetSource() const;

};
}
}

#endif //ASAPO_FABRIC_WAITABLE_TASK_H
