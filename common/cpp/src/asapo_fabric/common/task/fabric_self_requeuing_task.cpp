#include "fabric_self_requeuing_task.h"
#include "../fabric_context_impl.h"

using namespace asapo;
using namespace fabric;

FabricSelfRequeuingTask::~FabricSelfRequeuingTask() {
    Stop();
}

FabricSelfRequeuingTask::FabricSelfRequeuingTask(FabricContextImpl* parentContext) : stop_response_future_{stop_response_.get_future()} {
    parent_context_ = parentContext;
}

void FabricSelfRequeuingTask::Start() {
    if (was_queued_already_) {
        throw std::runtime_error("FabricSelfRequeuingTask can only be queued once");
    }
    was_queued_already_ = true;
    RequeueSelf();
}

void FabricSelfRequeuingTask::Stop() {
    if (was_queued_already_ && still_running_) {
        still_running_ = false;
        fi_cancel(&parent_context_->endpoint_->fid, this);
        stop_response_future_.wait();
    }
}

void FabricSelfRequeuingTask::HandleCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) {
    OnCompletion(entry, source);
    AfterCompletion();
}

void FabricSelfRequeuingTask::HandleErrorCompletion(const fi_cq_err_entry* errEntry) {
    // If we are not running and got a FI_ECANCELED its probably expected.
    if (still_running_ || errEntry->err != FI_ECANCELED) {
        OnErrorCompletion(errEntry);
    }
    AfterCompletion();
}

void FabricSelfRequeuingTask::AfterCompletion() {
    if (still_running_) {
        RequeueSelf();
    } else {
        stop_response_.set_value();
    }
}

FabricContextImpl* FabricSelfRequeuingTask::ParentContext() {
    return parent_context_;
}
