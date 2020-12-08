#ifndef ASAPO_SHARED_EVENT_LIST_H
#define ASAPO_SHARED_EVENT_LIST_H

#include <string>
#include <vector>
#include <list>
#include <mutex>
#include <windows.h>
#include <chrono>
#include "asapo/common.h"

namespace asapo {

const uint64_t kFileDelayMs = 500;

struct SingleEvent {
    std::string file_name;
    std::chrono::system_clock::time_point time;
    bool apply_delay;
};

class SharedEventList {
  public:
    FilesToSend GetAndClearEvents();
    void AddEvent(std::string event, bool apply_delay);
  private:
    std::mutex mutex_;
    std::list<SingleEvent> events_;
};

}

#endif //ASAPO_SHARED_EVENT_LIST_H
