#include "shared_event_list.h"

#include <algorithm>

using std::chrono::high_resolution_clock;

namespace asapo {

FilesToSend SharedEventList::GetAndClearEvents() {
    std::lock_guard<std::mutex> lock(mutex_);
    FilesToSend events;

    for (auto it = events_.begin(); it != events_.end(); /* NOTHING */) {
        uint64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>( high_resolution_clock::now() -
            it->time).count();
        if (elapsed_ms > 1000) {
            events.push_back(it->file_name);
            it = events_.erase(it);
        } else {
            ++it;
        }
    }

    return events;
}
void SharedEventList::AddEvent(std::string event) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto findIter = std::find_if(events_.begin(), events_.end(),  [&]( const SingleEvent& e ){ return e.file_name == event;});
    if ( events_.end() == findIter ) {
        events_.emplace_back(SingleEvent{std::move(event),high_resolution_clock::now()});
    } else {
        findIter->time = high_resolution_clock::now();
    }
}
}
