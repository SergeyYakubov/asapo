#ifndef ASAPO_FABRIC_HANDSHAKE_ACCEPTING_TASK_H
#define ASAPO_FABRIC_HANDSHAKE_ACCEPTING_TASK_H

#include "../../common/task/fabric_task.h"
#include "../../common/fabric_context_impl.h"

namespace asapo {
namespace fabric {

// Need forward declaration for reference inside the task
class FabricServerImpl;

/**
 * This task will automatically requeue itself
 */
class FabricHandshakeAcceptingTask : public FabricTask {

  private:
    FabricServerImpl* server_;
    FabricHandshakePayload handshake_payload_{};
  public:
    ~FabricHandshakeAcceptingTask();
    explicit FabricHandshakeAcceptingTask(FabricServerImpl* server);

    void HandleCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) override;
    void HandleErrorCompletion(const fi_cq_err_entry* errEntry) override;

    void StartRequest();
    void DeleteRequest();

  private:
    void HandleAccept(Error* error);
    void OnError(Error* error);
};

}
}

#endif //ASAPO_FABRIC_HANDSHAKE_ACCEPTING_TASK_H
