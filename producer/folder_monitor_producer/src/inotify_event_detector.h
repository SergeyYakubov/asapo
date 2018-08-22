#ifndef ASAPO_INOTOFY_EVENT_DETECTOR_H
#define ASAPO_INOTOFY_EVENT_DETECTOR_H

#include "event_detector.h"
#include "foldermon_config.h"

namespace asapo {

class InotifyEventDetector : public AbstractEventDetector {
  public:
    Error GetNextEvent(EventHeader* event_header) override;
    InotifyEventDetector(const FolderMonConfig* config);
  private:
    const FolderMonConfig* config_;
};

}

#endif //ASAPO_INOTOFY_EVENT_DETECTOR_H
