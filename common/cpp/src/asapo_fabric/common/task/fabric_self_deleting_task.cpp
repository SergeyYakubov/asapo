#include "fabric_self_deleting_task.h"

void asapo::fabric::FabricSelfDeletingTask::HandleCompletion(const fi_cq_tagged_entry*, FabricAddress) {
    OnDone();
}

void asapo::fabric::FabricSelfDeletingTask::HandleErrorCompletion(const fi_cq_err_entry*) {
    OnDone();
}

void asapo::fabric::FabricSelfDeletingTask::OnDone() {
    delete this;
}
