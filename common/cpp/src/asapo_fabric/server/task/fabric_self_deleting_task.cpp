#include "fabric_self_deleting_task.h"

void asapo::fabric::FabricSelfDeletingTask::HandleCompletion(const fi_cq_tagged_entry* entry,
        asapo::fabric::FabricAddress source) {
    OnDone();
}

void asapo::fabric::FabricSelfDeletingTask::HandleErrorCompletion(fi_cq_err_entry* errEntry) {
    OnDone();
}

void asapo::fabric::FabricSelfDeletingTask::OnDone() {
    delete this;
}
