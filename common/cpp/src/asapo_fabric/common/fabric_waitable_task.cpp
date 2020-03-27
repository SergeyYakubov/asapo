#include "fabric_waitable_task.h"
#include "../fabric_internal_error.h"

using namespace asapo;
using namespace fabric;

FabricWaitableTask::FabricWaitableTask() : future_{promise_.get_future()} {

}

void FabricWaitableTask::HandleCompletion(const fi_cq_tagged_entry* entry, FabricAddress source) {
    source_ = source;
    promise_.set_value();
}

void FabricWaitableTask::HandleErrorCompletion(fi_cq_err_entry* errEntry) {
    error_ = ErrorFromFabricInternal("FabricWaitableTask", -errEntry->err);
    promise_.set_value();
}

void FabricWaitableTask::Wait(Error* error) {
    future_.wait();
    error->swap(error_);
}

FabricAddress FabricWaitableTask::GetSource() const {
    return source_;
}
