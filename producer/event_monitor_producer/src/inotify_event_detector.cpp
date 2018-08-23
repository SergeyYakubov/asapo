#include "inotify_event_detector.h"

namespace asapo {

Error InotifyEventDetector::GetNextEvent(EventHeader* event_header) {
    return nullptr;
}
InotifyEventDetector::InotifyEventDetector(const EventMonConfig* config) : config_{config} {
}

}