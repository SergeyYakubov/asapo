#include "fabric_recv_any_task.h"

using namespace asapo;
using namespace fabric;

void FabricRecvAnyTask::HandleCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) {
    messageId_ = entry->tag;
    FabricWaitableTask::HandleCompletion(entry, source);
}

void FabricRecvAnyTask::HandleErrorCompletion(fi_cq_err_entry* errEntry) {
    messageId_ = errEntry->tag;
    FabricWaitableTask::HandleErrorCompletion(errEntry);
}

FabricMessageId FabricRecvAnyTask::GetMessageId() const {
    return messageId_;
}
