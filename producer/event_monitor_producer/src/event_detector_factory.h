#ifndef ASAPO_EVENT_DETECTOR_FACTORY_H
#define ASAPO_EVENT_DETECTOR_FACTORY_H

#include "event_detector.h"
#include "asapo/common/error.h"

namespace asapo {

class EventDetectorFactory {
  public:
    EventDetector CreateEventDetector();
};

}

#endif //ASAPO_EVENT_DETECTOR_FACTORY_H
