#ifndef ASAPO_EventMonitor_ERROR_H
#define ASAPO_EventMonitor_ERROR_H

#include "asapo/common/error.h"

namespace asapo {

enum class EventMonitorErrorType {
    kNoNewEvent,
    kSystemError
};

using EventMonitorErrorTemplate = ServiceErrorTemplate<EventMonitorErrorType>;

namespace EventMonitorErrorTemplates {
auto const kNoNewEvent = EventMonitorErrorTemplate {
    "no new event", EventMonitorErrorType::kNoNewEvent
};

auto const kSystemError = EventMonitorErrorTemplate {
    "system error", EventMonitorErrorType::kSystemError
};



}
}

#endif //ASAPO_EventMonitor_ERROR_H

