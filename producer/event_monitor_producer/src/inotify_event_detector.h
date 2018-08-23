#ifndef ASAPO_INOTOFY_EVENT_DETECTOR_H
#define ASAPO_INOTOFY_EVENT_DETECTOR_H

#include "event_detector.h"
#include "eventmon_config.h"

namespace asapo {

class InotifyEventDetector : public AbstractEventDetector {
  public:
    Error GetNextEvent(EventHeader* event_header) override;
    InotifyEventDetector(const EventMonConfig* config);
  private:
    const EventMonConfig* config_;
};

}

#endif //ASAPO_INOTOFY_EVENT_DETECTOR_H
