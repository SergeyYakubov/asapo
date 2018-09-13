#ifndef ASAPO_SHARED_EVENT_LIST_H
#define ASAPO_SHARED_EVENT_LIST_H

#include <string>
#include <vector>
#include <list>
#include <mutex>
#include <windows.h>
#include <chrono>
#include "common.h"

namespace asapo {

struct SingleEvent {
  std::string file_name;
  std::chrono::high_resolution_clock::time_point time;
};

class SharedEventList {
 public:
  FilesToSend GetAndClearEvents();
  void AddEvent(std::string event);
 private:
  std::mutex mutex_;
  std::list<SingleEvent> events_;
};

}

#endif //ASAPO_SHARED_EVENT_LIST_H
