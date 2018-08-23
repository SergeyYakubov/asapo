#include "event_detector_factory.h"

#include "eventmon_config.h"
#include "inotify_event_detector.h"

namespace asapo {

EventDetector EventDetectorFactory::CreateEventDetector() {
    auto config = GetEventMonConfig();
    return EventDetector{new InotifyEventDetector(config)};
}

}