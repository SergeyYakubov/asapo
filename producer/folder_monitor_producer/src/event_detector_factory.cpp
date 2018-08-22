#include "event_detector_factory.h"

#include "foldermon_config.h"
#include "inotify_event_detector.h"

namespace asapo {

EventDetector EventDetectorFactory::CreateEventDetector() {
    auto config = GetFolderMonConfig();
    return EventDetector{new InotifyEventDetector(config)};
}

}