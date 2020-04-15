#include "event_detector_factory.h"

#include "eventmon_config.h"
#include "folder_event_detector.h"

namespace asapo {

EventDetector EventDetectorFactory::CreateEventDetector() {
    auto config = GetEventMonConfig();
    return EventDetector{new FolderEventDetector(config)};
}

}