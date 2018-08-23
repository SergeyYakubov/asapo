#ifndef ASAPO_EVENT_DETECTOR_H
#define ASAPO_EVENT_DETECTOR_H

#include <memory>
#include "producer/common.h"

namespace asapo {

class AbstractEventDetector {
  public:
    virtual Error GetNextEvent(EventHeader* event_header) = 0;
};

using EventDetector = std::unique_ptr<AbstractEventDetector>;

}

#endif //ASAPO_EVENT_DETECTOR_H
