#include <rdma/fi_tagged.h>
#include "fabric_alive_check_response_task.h"
#include "../fabric_context_impl.h"

using namespace asapo;
using namespace fabric;

void FabricAliveCheckResponseTask::RequeueSelf() {
    Error tmpError = nullptr;

    ParentContext()->HandleRawFiCommand(this, &tmpError,
                                        fi_trecv, nullptr, 0, nullptr, FI_ADDR_UNSPEC, FI_ASAPO_TAG_ALIVE_CHECK, kRecvTaggedExactMatch);

    // Error is ignored
}

void FabricAliveCheckResponseTask::OnCompletion(const fi_cq_tagged_entry*, FabricAddress) {
    // We received a ping, LibFabric will automatically notify the sender about the completion.
}

void FabricAliveCheckResponseTask::OnErrorCompletion(const fi_cq_err_entry*) {
    // Error is ignored
}

FabricAliveCheckResponseTask::FabricAliveCheckResponseTask(FabricContextImpl* parentContext)
    : FabricSelfRequeuingTask(parentContext) {

}
