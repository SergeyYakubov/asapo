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



using FilesToSend = std::vector<std::string>;

}

#endif //ASAPO_EVENT_MONITOR_PRODUCER_COMMON_H
