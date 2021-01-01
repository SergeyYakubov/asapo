#include <asapo/common/io_error.h>
#include "fabric_waitable_task.h"
#include "../../fabric_internal_error.h"

using namespace asapo;
using namespace fabric;

FabricWaitableTask::FabricWaitableTask() : future_{promise_.get_future()}, source_{FI_ADDR_NOTAVAIL} {

}

void FabricWaitableTask::HandleCompletion(const fi_cq_tagged_entry*, FabricAddress source) {
    source_ = source;
    promise_.set_value();
}

void FabricWaitableTask::HandleErrorCompletion(const fi_cq_err_entry* errEntry) {
    error_ = ErrorFromFabricInternal("FabricWaitableTask", -errEntry->err);
    promise_.set_value();
}

void FabricWaitableTask::Wait(uint32_t sleepInMs, Error* error) {
    if (sleepInMs) {
        if (future_.wait_for(std::chrono::milliseconds(sleepInMs)) == std::future_status::timeout) {
            *error = IOErrorTemplates::kTimeout.Generate();
            return;
        }
    } else {
        future_.wait();
    }
    error->swap(error_);
}

FabricAddress FabricWaitableTask::GetSource() const {
    return source_;
}
