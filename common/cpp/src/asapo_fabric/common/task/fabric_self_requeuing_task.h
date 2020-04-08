#ifndef ASAPO_FABRIC_SELF_REQUEUING_TASK_H
#define ASAPO_FABRIC_SELF_REQUEUING_TASK_H

#include <future>
#include "fabric_task.h"

namespace asapo {
namespace fabric {
class FabricContextImpl;

class FabricSelfRequeuingTask : public FabricTask {
  private:
    FabricContextImpl* parent_context_;
    volatile bool still_running_ = true;
    bool was_queued_already_ = false;
    std::promise<void> stop_response_;
    std::future<void> stop_response_future_;
  public:
    ~FabricSelfRequeuingTask();
    explicit FabricSelfRequeuingTask(FabricContextImpl* parentContext);

    void Start();
    void Stop();
  public:
    void HandleCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) final;
    void HandleErrorCompletion(const fi_cq_err_entry* errEntry) final;
  protected:
    FabricContextImpl* ParentContext();

    virtual void RequeueSelf() = 0;
    virtual void OnCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) = 0;
    virtual void OnErrorCompletion(const fi_cq_err_entry* errEntry) = 0;
  private:
    void AfterCompletion();
};
}
}


#endif //ASAPO_FABRIC_SELF_REQUEUING_TASK_H
