#ifndef ASAPO_FABRIC_HANDSHAKE_ACCEPTING_TASK_H
#define ASAPO_FABRIC_HANDSHAKE_ACCEPTING_TASK_H

#include "../../common/task/fabric_task.h"
#include "../../common/fabric_context_impl.h"

namespace asapo {
namespace fabric {

// Need forward declaration for reference inside the task
class FabricServerImpl;

class FabricHandshakeAcceptingTask : public FabricSelfRequeuingTask {
  private:
    FabricHandshakePayload handshake_payload_{};

  public:
    explicit FabricHandshakeAcceptingTask(FabricServerImpl* server);

  private:
    FabricServerImpl* ServerContext();

  protected: // override FabricSelfRequeuingTask
    void RequeueSelf() override;

    void OnCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) override;

    void OnErrorCompletion(const fi_cq_err_entry* errEntry) override;

  private:
    void HandleAccept(Error* error);
    void OnError(const Error* error);
};

}
}

#endif //ASAPO_FABRIC_HANDSHAKE_ACCEPTING_TASK_H
