#ifndef ASAPO_EVENT_MONITOR_PRODUCER_COMMON_H
#define ASAPO_EVENT_MONITOR_PRODUCER_COMMON_H

#include <cstdint>
#include <string>
#include <vector>

namespace asapo {

enum class EventType {
  closed,
  renamed_to
};

struct FileEvent {
  EventType type;
  uint64_t size;
  std::string name;
};

using FileEvents = std::vector<FileEvent>;

}

#endif //ASAPO_EVENT_MONITOR_PRODUCER_COMMON_H
