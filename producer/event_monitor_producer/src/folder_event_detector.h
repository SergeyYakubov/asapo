#ifndef ASAPO_INOTOFY_EVENT_DETECTOR_H
#define ASAPO_INOTOFY_EVENT_DETECTOR_H

#include "event_detector.h"
#include "eventmon_config.h"
#include "io/io.h"
#include "logger/logger.h"

#include "system_folder_watch.h"

namespace asapo {

class FolderEventDetector : public AbstractEventDetector {
  public:
    Error GetNextEvent(EventHeader* event_header) override;
    FolderEventDetector(const EventMonConfig* config);
    std::unique_ptr<SystemFolderWatch> system_folder_watch__;
    const AbstractLogger* log__;
  private:
    const EventMonConfig* config_;
};

}

#endif //ASAPO_INOTOFY_EVENT_DETECTOR_H
