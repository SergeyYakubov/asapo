#include "shared_event_list.h"

namespace asapo {

FilesToSend SharedEventList::GetAndClearEvents() {
    std::lock_guard<std::mutex> lock(mutex_);
    FilesToSend events = std::move(events_);
    events_.clear();
    return events;
}
void SharedEventList::AddEvent(std::string event) {
    std::lock_guard<std::mutex> lock(mutex_);
    events_.emplace_back(std::move(event));
}
}
