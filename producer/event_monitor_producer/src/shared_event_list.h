#ifndef ASAPO_SHARED_EVENT_LIST_H
#define ASAPO_SHARED_EVENT_LIST_H

#include <string>
#include <vector>
#include <mutex>

#include "common.h"

namespace asapo {

class SharedEventList {
 public:
  FilesToSend GetAndClearEvents();
  void AddEvent(std::string event);
 private:
  std::mutex mutex_;
  FilesToSend events_;
};

}

#endif //ASAPO_SHARED_EVENT_LIST_H
