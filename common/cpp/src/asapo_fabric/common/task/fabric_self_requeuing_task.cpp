#include "fabric_self_requeuing_task.h"
#include "../fabric_context_impl.h"

using namespace asapo;
using namespace fabric;

FabricSelfRequeuingTask::~FabricSelfRequeuingTask() {
    Stop();
}

FabricSelfRequeuingTask::FabricSelfRequeuingTask(FabricContextImpl* parentContext) {
    parent_context_ = parentContext;
}

void FabricSelfRequeuingTask::Start() {
    if (was_queued_already_) {
        throw std::runtime_error("FabricSelfRequeuingTask can only be queued once");
    }
    RequeueSelf(parent_context_);
}

void FabricSelfRequeuingTask::Stop() {
    if (was_queued_already_ && still_running_) {
        still_running_ = false;
        printf("Going to stop FabricSelfRequeuingTask!!!");
        fi_cancel(&parent_context_->endpoint_->fid, this);
        stop_response_future_.wait();
    }
}

void FabricSelfRequeuingTask::HandleCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) {
    OnCompletion(entry, source);
    AfterCompletion();
}

void FabricSelfRequeuingTask::HandleErrorCompletion(const fi_cq_err_entry* errEntry) {
    OnErrorCompletion(errEntry);
    AfterCompletion();
}

void FabricSelfRequeuingTask::AfterCompletion() {
    if (still_running_) {
        RequeueSelf(parent_context_);
    } else {
        stop_response_.set_value();
    }
}
