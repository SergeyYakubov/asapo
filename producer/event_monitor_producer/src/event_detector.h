#ifndef ASAPO_EVENT_DETECTOR_H
#define ASAPO_EVENT_DETECTOR_H

#include <memory>
#include "asapo_producer.h"

namespace asapo {

class AbstractEventDetector {
  public:
    virtual Error GetNextEvent(EventHeader* event_header) = 0;
    virtual Error StartMonitoring() = 0;
    virtual ~AbstractEventDetector() = default;
};

using EventDetector = std::unique_ptr<AbstractEventDetector>;

}

#endif //ASAPO_EVENT_DETECTOR_H
